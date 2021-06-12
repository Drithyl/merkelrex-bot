
#include <iostream>
#include <iomanip>
#include <cmath>
#include "../headers/PrecisionString.h"
#include "../headers/StandardDeviationStrategy.h"

StandardDeviationStrategy::StandardDeviationStrategy(
    Wallet* walletPointer,
    std::string _askFundsCurrency,
    std::string _bidFundsCurrency,
    unsigned int requiredNbrOfObservedEntriesToTrade, 
    unsigned int nbrOfObservedEntries
)
:   wallet(walletPointer),
    askFundsCurrency(_askFundsCurrency),
    bidFundsCurrency(_bidFundsCurrency),
    minimumNbrOfObservedEntriesToTrade(requiredNbrOfObservedEntriesToTrade),
    nbrOfEntriesToObserve(nbrOfObservedEntries)
{
    //Initialize our product based on the two currencies we received
    product = askFundsCurrency + "/" + bidFundsCurrency;
}

// Updates the means, variances and deviations of our list of observed asks and bids with the newly received ones,
// as well as the lowest ask and highest bid in these new asks and bids
void StandardDeviationStrategy::updateMetrics(std::list<std::shared_ptr<OrderBookEntry>>& asks, std::list<std::shared_ptr<OrderBookEntry>>& bids)
{
    // Insert all new asks and bids into our observed list
    observedAsks.insert(observedAsks.end(), asks.begin(), asks.end());
    observedBids.insert(observedBids.end(), bids.begin(), bids.end());

    // If this makes our lists bigger than the limit that was previously set,
    // then remove the older asks and bids that are at the front of the lists
    if (observedAsks.size() > nbrOfEntriesToObserve)
        observedAsks.erase(observedAsks.begin(), std::next(observedAsks.begin(), std::min(observedAsks.size() - nbrOfEntriesToObserve, observedAsks.size())));
    
    if (observedBids.size() > nbrOfEntriesToObserve)
        observedBids.erase(observedBids.begin(), std::next(observedBids.begin(), std::min(observedBids.size() - nbrOfEntriesToObserve, observedBids.size())));

    // Calculate mean, variance and deviations for asks
    asksMean = getMean(observedAsks);
    asksVariance = getVariance(asksMean, observedAsks);

    asksOneStdDeviation = sqrt(asksVariance);
    asksTwoStdDeviation = asksOneStdDeviation * 2;

    // Find lowest ask in the newly received ones and store it as the current one (at the front)
    std::shared_ptr<OrderBookEntry> lowestAsk = OrderBook::getLowPriceEntry(asks);
    lowestAsks.insert(lowestAsks.begin(), lowestAsk);

    // Calculate mean, variance and deviations for bids
    bidsMean = getMean(observedBids);
    bidsVariance = getVariance(bidsMean, observedBids);

    bidsOneStdDeviation = sqrt(bidsVariance);
    bidsTwoStdDeviation = bidsOneStdDeviation * 2;

    // Find highest bid in the newly received ones and store it as the current one (at the front)
    std::shared_ptr<OrderBookEntry> highestBid = OrderBook::getHighPriceEntry(bids);
    highestBids.insert(highestBids.begin(), highestBid);
}

// Cleans or withdraws bot orders that are empty or have not been fulfilled,
// and returns a list of all the ones which were withdrawn (or empty if none was)
std::list<OrderBookEntry> StandardDeviationStrategy::cleanPendingOrders(std::string currentTimestamp)
{
    auto it = pendingAsks.begin();
    std::list<OrderBookEntry> withdrawnOrders;

    // Iterate through our pending asks
    while (it != pendingAsks.end())
    {
        // If it's empty, remove it
        if ((*it)->amount <= 0.000000001f)
            it = pendingAsks.erase(it);

        // If it was not fulfilled in the previous timestamp, or was only fulfilled partially, withdraw it
        // if the price is too high compared to our new metrics
        else if ((*it)->price > asksMean - asksOneStdDeviation)
        {
            double fundsToUnlock = (*it)->amount;

            try
            {
                // Unlock the funds that were locked for this order, since it didn't get fulfilled completely and got withdrawn
                wallet->unlockCurrency(askFundsCurrency, fundsToUnlock);
            }
            catch(const std::exception& e)
            {
                std::cerr << "Could not unlock " << std::to_string(fundsToUnlock) << " " << askFundsCurrency << " ask funds: " << e.what() << '\n';
            }

            // Copy the order to withdraw for logging purposes, so the amount does not show up as 0 in the log
            withdrawnOrders.insert(withdrawnOrders.begin(), OrderBookEntry{
                (*it)->price, 
                (*it)->amount, 
                (*it)->timestamp, 
                (*it)->product, 
                (*it)->orderType}
            );
            
            // Set amount to 0 so it will not be matched in the next iteration of the matching engine, but will get removed instead
            (*it)->amount = 0;
            it = pendingAsks.erase(it);
        }

        else it++;
    }

    it = pendingBids.begin();

    // Iterate through our pending bids
    while (it != pendingBids.end())
    {
        // If it's empty, remove it
        if ((*it)->amount <= 0.000000001f)
            it = pendingBids.erase(it);

        // If it was not fulfilled in the previous timestamp,
        // or was only fulfilled partially, withdraw it
        else if ((*it)->price < bidsMean + bidsOneStdDeviation)
        {
            double fundsToUnlock = (*it)->amount * (*it)->price;

            try
            {
                // Unlock the funds that were locked for this order, since it didn't get fulfilled completely and got withdrawn
                // Since this is a bid, we unlock as many funds as the remaining amount of the order times the price per unit
                wallet->unlockCurrency(bidFundsCurrency, fundsToUnlock);
            }
            catch(const std::exception& e)
            {
                std::cerr << "Could not unlock " << to_string_with_precision(fundsToUnlock, 20) << " " << bidFundsCurrency << " bid funds: " << e.what() << '\n';
            }
            
            // Copy the order to withdraw for logging purposes, so the amount does not show up as 0 in the log
            withdrawnOrders.insert(withdrawnOrders.begin(), OrderBookEntry{
                (*it)->price, 
                (*it)->amount, 
                (*it)->timestamp, 
                (*it)->product, 
                (*it)->orderType}
            );

            // Set amount to 0 so it will not be matched in the next iteration of the matching engine, but will get removed instead
            (*it)->amount = 0;
            it = pendingBids.erase(it);
        }

        else it++;
    }

    return withdrawnOrders;
}

// Use our most recent metrics to decide whether to place new orders and if so, 
// insert them into the listOfOrdersPlaced that is passed by reference
void StandardDeviationStrategy::tradeCurrentTimestamp(std::list<std::shared_ptr<OrderBookEntry>>& listOfOrdersPlaced)
{
    double askFunds = wallet->getCurrentFunds(askFundsCurrency);
    double bidFunds = wallet->getCurrentFunds(bidFundsCurrency);

    // Not enough data to make accurate predictions
    if (observedAsks.size() < minimumNbrOfObservedEntriesToTrade)
    {
        std::cout << "Not enough " << product << " metrics to trade; have " << std::to_string(observedAsks.size()) << ", need " << minimumNbrOfObservedEntriesToTrade << std::endl;
        return;
    }

    // Bid signal; buy as much product as the lowest ask offers
    if (isBidSignal() == true && bidFunds > 0.000000001f)
    {
        //std::cout << "Bid signal for " << product << std::endl;
        this->placeBid(listOfOrdersPlaced);
    }

    // Ask signal; sell as much product as the highest bid requests
    else if (isAskSignal() == true && askFunds > 0.000000001f)
    {
        //std::cout << "Ask signal for " << product << std::endl;
        this->placeAsk(listOfOrdersPlaced);
    }

    //else std::cout << "No trading done." << std::endl;
}

// Track order to withdraw or remove it later if needed, in the cleanPendingOrders() function
void StandardDeviationStrategy::trackOrder(std::shared_ptr<OrderBookEntry>& pointerToOrder)
{
    if (pointerToOrder->orderType == OrderBookType::ask)
        pendingAsks.push_back(pointerToOrder);

    else if (pointerToOrder->orderType == OrderBookType::bid)
        pendingBids.push_back(pointerToOrder);
}

double StandardDeviationStrategy::getMean(std::list<std::shared_ptr<OrderBookEntry>>& elements)
{
    double mean = 0;

    if (elements.size() == 0)
        return 0;

    for (std::shared_ptr<OrderBookEntry> entry : elements)
    {
        mean += entry->price;
    }

    return mean / elements.size();
}

double StandardDeviationStrategy::getVariance(double mean, std::list<std::shared_ptr<OrderBookEntry>>& elements)
{
    double variance = 0;

    if (elements.size() == 0)
        return 0;

    for (std::shared_ptr<OrderBookEntry>& entry : elements)
    {
        variance += pow( entry->price - mean, 2 );
    }

    return variance / elements.size();
}

bool StandardDeviationStrategy::isBidSignal()
{
    // If lowest ask before current one is higher (price is falling), and current one has crossed the -1 deviation, then we buy.
    return lowestAsks[1]->price > lowestAsks[0]->price && lowestAsks[0]->price <= asksMean - asksOneStdDeviation;
}

bool StandardDeviationStrategy::isAskSignal()
{
    // If highest bid before current one is lower (price is on the rise), and current one has crossed the +1 deviation, then we sell.
    return highestBids[1]->price < highestBids[0]->price && highestBids[0]->price >= bidsMean + bidsOneStdDeviation;
}

// Place a bid using the lowest ask price we can find
void StandardDeviationStrategy::placeBid(std::list<std::shared_ptr<OrderBookEntry>>& listOfOrdersPlaced)
{
    // Get our current funds to bid with
    double bidFunds = wallet->getCurrentFunds(bidFundsCurrency);

    // Get the price that we need to bid at
    double priceToBidAt = lowestAsks[0]->price;

    // Determine the amount to buy based on our funds, and always buy only 20% of the full amount
    double amountToBuy = std::min(lowestAsks[0]->amount, bidFunds / priceToBidAt) * .2f;
    double totalPriceToPay = amountToBuy * priceToBidAt;

    // To avoid float point precision, we will ignore any trades which would involve tiny amounts
    if (amountToBuy <= 0.000000001f)
        return;

    std::shared_ptr<OrderBookEntry> pointerToBid = std::make_shared<OrderBookEntry>(
        priceToBidAt, 
        amountToBuy, 
        lowestAsks[0]->timestamp,
        product, 
        OrderBookType::bid,
        "bot"
    );

    // Lock the currency used for this bid so we don't accidentally use it again in a different order
    wallet->lockCurrency(bidFundsCurrency, totalPriceToPay);

    // Push our order into the list passed by reference
    listOfOrdersPlaced.push_back(pointerToBid);
    pendingBids.push_back(pointerToBid);
}

// Place an ask using the highest bid price we can find
void StandardDeviationStrategy::placeAsk(std::list<std::shared_ptr<OrderBookEntry>>& listOfOrdersPlaced)
{
    // Get our current funds to ask with
    double askFunds = wallet->getCurrentFunds(askFundsCurrency);
    
    // Get the price that we need to ask
    double priceToAsk = highestBids[0]->price;

    // Determine the amount to sell based on our funds, and always sell only 20% of the full amount
    double amountToSell = std::min(askFunds, highestBids[0]->amount) * .2f;

    // To avoid float point precision, we will ignore any trades which would involve tiny amounts
    if (amountToSell <= 0.000000001f)
        return;

    // Create our ask object
    std::shared_ptr<OrderBookEntry> pointerToAsk = std::make_shared<OrderBookEntry>(
        priceToAsk, 
        amountToSell, 
        highestBids[0]->timestamp, 
        product, 
        OrderBookType::ask,
        "bot"
    );

    // Lock the currency used for this ask so we don't accidentally use it again in a different order
    wallet->lockCurrency(askFundsCurrency, amountToSell);
    
    // Push our order into the list passed by reference
    listOfOrdersPlaced.push_back(pointerToAsk);
    pendingAsks.push_back(pointerToAsk);
}