
#include "../headers/OrderBook.h"
#include "../headers/CSVReader.h"
#include "../headers/PrecisionString.h"
#include <map>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <ctime>

/** constructor, reading a csv data file */
OrderBook::OrderBook(std::string filename)
{
    orders = CSVReader::readCSV(filename);
    currentTime = this->getEarliestTime();
    std::map<std::string, bool> productMap;
    std::list<std::shared_ptr<OrderBookEntry>>::iterator it = orders.begin();

    // Stores the current timestamp that we're iterating through, to
    // document the start index of all new timestamps in the data for future use
    std::string currentTimestampIteration = "";

    // Initialize map entries with all product types, as well 
    // as the boundaries for the entries in the first timestap
    while (it != orders.end())
    {
        // A new timestamp was reached in the data; store the index at which it's found
        if ((*it)->timestamp != currentTimestampIteration)
        {
            timestampIterators.push_back(it);
            currentTimestampIteration = (*it)->timestamp;
        }

        productMap[(*it)->product] = true;
        it++;
    }

    for (auto const& entry : productMap)
    {
        // Entry.first is the map key, and entry.last is the actual value of the entry, a boolean
        // push all the known products to our class private member to store and not have to look up again
        knownProducts.push_back(entry.first);
    }
}

/** return vector of all known products in the dataset */
std::vector<std::string> OrderBook::getKnownProducts()
{
    std::vector<std::string> copiedVector(knownProducts);
    return copiedVector;
}

/** return list of Orders according to the sent filters in the current timestamp */
std::list<std::shared_ptr<OrderBookEntry>> OrderBook::getCurrentOrders(OrderBookType type, std::string product)
{
    std::list<std::shared_ptr<OrderBookEntry>> orders_sub;

    // Fetch the iterator that corresponds to our current timestamp position
    std::list<std::shared_ptr<OrderBookEntry>>::iterator startOfCurrentOrdersIt = timestampIterators[currentPositionInTimestampIterators];

    // Use a helper function to set an iterator to point to the end of this timestamp in the orders
    std::list<std::shared_ptr<OrderBookEntry>>::iterator endOfCurrentOrdersIt = getEndItOfCurrentTimestamp();

    // While we haven't reached the end of this timestamp, push the current order to the sublist
    while (startOfCurrentOrdersIt != endOfCurrentOrdersIt)
    {
        if ((*startOfCurrentOrdersIt)->orderType == type && (*startOfCurrentOrdersIt)->product == product)
            orders_sub.push_back(*startOfCurrentOrdersIt);
        
        startOfCurrentOrdersIt++;
    }

    return orders_sub;
}

std::string OrderBook::getEarliestTime()
{
    if (orders.size() <= 0)
        return "";

    return orders.front()->timestamp;
}

std::string OrderBook::getCurrentTimestamp()
{
    return currentTime;
}

// Go to next known timestamp in our vector of iterators and update the current time
std::string OrderBook::goToNextTimestamp()
{
    this->goToNextTimestampIndex();
    std::list<std::shared_ptr<OrderBookEntry>>::iterator nextTimestampItInOrders = timestampIterators[currentPositionInTimestampIterators];
    currentTime = (*nextTimestampItInOrders)->timestamp;
    return currentTime;
}

// Go to next index in our vector of iterators, and wrap around if we reached the end of it
void OrderBook::goToNextTimestampIndex()
{
    currentPositionInTimestampIterators++;

    // If our list of timestamp indexes does not have a next element, then wrap around and set ourselves back at the start
    if (currentPositionInTimestampIterators >= timestampIterators.size() - 1)
        currentPositionInTimestampIterators = 0;
}

// Insert an order at the end of the current timestamp, using the current iterator
std::shared_ptr<OrderBookEntry>& OrderBook::insertOrder(std::shared_ptr<OrderBookEntry>& order)
{
    std::list<std::shared_ptr<OrderBookEntry>>::iterator it = this->getEndItOfCurrentTimestamp();

    // Insert the order at the end of the current timestamp's orders
    orders.insert(it, order);

    return *it;
}

// Match only the asks and bids within the same timestamp, without considering
// leftover ones from previous timestamps. This is fast, 3 to 7s to complete all
std::list<OrderBookEntry> OrderBook::matchOnlyCurrentOrders(std::string product)
{
    // Fetch only the orders corresponding to the current timestamp
    std::list<std::shared_ptr<OrderBookEntry>> newAsks = getCurrentOrders(OrderBookType::ask, product);
    std::list<std::shared_ptr<OrderBookEntry>> newBids = getCurrentOrders(OrderBookType::bid, product);
    std::list<OrderBookEntry> sales;

    // Lowest bids (sells) will be matched to highest asks (buys) in priority
    // This sorting is trivial; it'll take less than 3 seconds for the entire order list
    newAsks.sort(OrderBookEntry::compareByPriceAsc);
    newBids.sort(OrderBookEntry::compareByPriceDesc);

    // Match only the new bids to the new asks, as the program originally did
    matchAsksToBids(newAsks, newBids, sales);

    return sales;
}

// Match current asks and bids also to the leftover ones from before. This is slower
// but also the way a currency exchange would work. Takes around 7 minutes to complete all
std::list<OrderBookEntry> OrderBook::matchCurrentAndPreviousOrders(std::string product)
{
    // Fetch only the orders corresponding to the current timestamp
    std::list<std::shared_ptr<OrderBookEntry>> newAsks = getCurrentOrders(OrderBookType::ask, product);
    std::list<std::shared_ptr<OrderBookEntry>> newBids = getCurrentOrders(OrderBookType::bid, product);
    std::list<OrderBookEntry> sales;

    // Lowest bids (sells) will be matched to highest asks (buys) in priority
    // This sorting is trivial; it'll take less than 3 seconds for the entire order list
    newAsks.sort(OrderBookEntry::compareByPriceAsc);
    newBids.sort(OrderBookEntry::compareByPriceDesc);

    // Create a list of older asks if one does not exist yet
    if (mapOfAsksTillCurrent.find(product) == mapOfAsksTillCurrent.end())
    {
        std::list<std::shared_ptr<OrderBookEntry>> listOfAsks;
        mapOfAsksTillCurrent.insert({ product, listOfAsks });
    }

    // Create a list of older bids if one does not exist yet
    if (mapOfBidsTillCurrent.find(product) == mapOfBidsTillCurrent.end())
    {
        std::list<std::shared_ptr<OrderBookEntry>> listOfBids;
        mapOfBidsTillCurrent.insert({ product, listOfBids });
    }

    // Assign a reference to the lists inside the maps for convenience
    std::list<std::shared_ptr<OrderBookEntry>>& asksTillCurrent = mapOfAsksTillCurrent.find(product)->second;
    std::list<std::shared_ptr<OrderBookEntry>>& bidsTillCurrent = mapOfBidsTillCurrent.find(product)->second;

    // Match only the new bids to the asks until (but not) the current ones
    matchAsksToBids(asksTillCurrent, newBids, sales);

    // Match only the new asks to the bids until (but not) the current ones
    matchAsksToBids(newAsks, bidsTillCurrent, sales);

    // Insert all new asks and bids together with the older ones after finishing the matching
    asksTillCurrent.insert(asksTillCurrent.end(), newAsks.begin(), newAsks.end());
    bidsTillCurrent.insert(bidsTillCurrent.end(), newBids.begin(), newBids.end());
    
    // Sort the entire list of asks and bids till now to preserve the ascending 
    // and descending order regardless of timestamp.
    asksTillCurrent.sort(OrderBookEntry::compareByPriceAsc);
    bidsTillCurrent.sort(OrderBookEntry::compareByPriceDesc);

    // An attempt to efficiently insert the newly fetched orders into the already sorted asks and bids
    // till current, using the lower_bound std method. If this code is enabled, the insert and sort lines 
    // above need to be commented out. This did not prove faster than those lines.
    //insertIntoSortedList(newAsks, asksTillCurrent, OrderBookEntry::compareByPriceAsc);
    //insertIntoSortedList(newBids, bidsTillCurrent, OrderBookEntry::compareByPriceDesc);

    return sales;
}

/** Insert a number of entries into a list that's already sorted using the lower_bound std method
 *  This proved slower than the std insert and sort methods by a fair bit (two times slower)
 */
void OrderBook::insertIntoSortedList(std::list<std::shared_ptr<OrderBookEntry>>& newEntries, std::list<std::shared_ptr<OrderBookEntry>>& sortedList, bool (*sortFunction)(const std::shared_ptr<OrderBookEntry>&, const std::shared_ptr<OrderBookEntry>&))
{
    std::list<std::shared_ptr<OrderBookEntry>>::iterator i = newEntries.begin();

    while (i != newEntries.end())
    {
        // Find the lower bound index at which the element could be inserted while preserving the order
        auto it = std::lower_bound(sortedList.begin(), sortedList.end(), *i, sortFunction);
        sortedList.insert(it, *i);
        i++;
    }
}

/** match a list of asks to bids using iterators, removing empty orders */
void OrderBook::matchAsksToBids(std::list<std::shared_ptr<OrderBookEntry>>& asks, std::list<std::shared_ptr<OrderBookEntry>>& bids, std::list<OrderBookEntry>& matches)
{
    std::list<std::shared_ptr<OrderBookEntry>>::iterator i = bids.begin();
    std::list<std::shared_ptr<OrderBookEntry>>::iterator j = asks.begin();

    while (i != bids.end())
    {
        while (j != asks.end())
        {
            // Break, as the provided lists are sorted, so every next iteration 
            // will result in unmatched orders, since the bid is now lower than the asks
            if ((*i)->price < (*j)->price)
                break;

            // Valid ask and bid, match them together to see if a sale is produced
            matchAskToBid(*j, *i, matches);

            // Remove empty ask and continue, since this bid still might have some amount left
            if ((*j)->amount <= 0.000000001f)
                j = asks.erase(j);

            // If ask not empty, continue with next ask
            else j++;

            // Remove empty bid, which sets the iterator to the next bid,
            // and restart the asks loop for this new bid
            if ((*i)->amount <= 0.000000001f)
            {
                i = bids.erase(i);
                j = asks.begin();
            }
        }

        // Move to next bid and reset the pointer at the beginning of the asks
        i++;
        j = asks.begin();
    }
}

void OrderBook::matchAskToBid(std::shared_ptr<OrderBookEntry>& ask, std::shared_ptr<OrderBookEntry>& bid, std::list<OrderBookEntry>& matches)
{
    std::shared_ptr<OrderBookEntry> salePointer = std::make_shared<OrderBookEntry>(
        ask->price, 
        0, 
        currentTime, 
        ask->product, 
        OrderBookType::asksale
    );

    // The user is placing a bid against the dataset, thus this will result in a bidsale
    if (bid->username != "dataset")
    {
        salePointer->username = "simuser";
        salePointer->orderType = OrderBookType::bidsale;
    }

    // The user is placing an ask against the dataset, thus this will result in an asksale
    else if (ask->username != "dataset")
    {
        salePointer->username = "simuser";
        salePointer->orderType = OrderBookType::asksale;
    }

    // Both bid and ask are fully matched; both get wiped
    if (bid->amount == ask->amount && ask->amount > 0.0000000001f)
    {
        salePointer->amount = ask->amount;
        matches.push_back(*salePointer);

        bid->amount = 0;
        ask->amount = 0;
    }

    // More product being bought than sold; bid gets sliced by the ask amount, and ask gets wiped
    else if (bid->amount > ask->amount && ask->amount > 0.0000000001f)
    {
        salePointer->amount = ask->amount;
        matches.push_back(*salePointer);

        bid->amount -= ask->amount;
        ask->amount = 0;
    }

    // More product being sold than bought; bid gets wiped and ask gets sliced by the bid amount
    else if (bid->amount < ask->amount && bid->amount > 0.0000000001f)
    {
        salePointer->amount = bid->amount;
        matches.push_back(*salePointer);

        ask->amount -= bid->amount;
        bid->amount = 0;
    }
}

double OrderBook::getHighPrice(std::list<std::shared_ptr<OrderBookEntry>>& orderSubset)
{
    double max = orderSubset.front()->price;

    for (std::shared_ptr<OrderBookEntry>& entry : orderSubset)
        if (entry->price > max)
            max = entry->price;

    return max;
}

double OrderBook::getLowPrice(std::list<std::shared_ptr<OrderBookEntry>>& orderSubset)
{
    double min = orderSubset.front()->price;

    for (std::shared_ptr<OrderBookEntry>& entry : orderSubset)
        if (entry->price < min)
            min = entry->price;

    return min;
}

std::shared_ptr<OrderBookEntry> OrderBook::getLowPriceEntry(std::list<std::shared_ptr<OrderBookEntry>>& orderSubset)
{
    std::shared_ptr<OrderBookEntry> minEntry = orderSubset.front();

    for (std::shared_ptr<OrderBookEntry> entry : orderSubset)
        if (entry->price < minEntry->price)
            minEntry = entry;

    return minEntry;
}

std::shared_ptr<OrderBookEntry> OrderBook::getHighPriceEntry(std::list<std::shared_ptr<OrderBookEntry>>& orderSubset)
{
    std::shared_ptr<OrderBookEntry> maxEntry = orderSubset.front();

    for (std::shared_ptr<OrderBookEntry> entry : orderSubset)
        if (entry->price > maxEntry->price)
            maxEntry = entry;

    return maxEntry;
}

std::list<std::shared_ptr<OrderBookEntry>>::iterator OrderBook::getEndItOfCurrentTimestamp()
{
    // Set the default at the last index of the entire order list
    std::list<std::shared_ptr<OrderBookEntry>>::iterator it = orders.end();

    // If there is a next index available in our list of timestamp indexes, then
    // that is the index at which the next timestamp begins, so return it minus 1
    if (currentPositionInTimestampIterators + 1 < timestampIterators.size() - 1)
        it = std::next(timestampIterators[currentPositionInTimestampIterators + 1], -1);

    return it;
}