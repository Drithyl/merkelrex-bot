

#pragma once
#include <vector>
#include <memory>
#include "Wallet.h"
#include "OrderBook.h"
#include "TradingStrategy.h"


class StandardDeviationStrategy
{
    public:

        /** Takes a pointer to a wallet of funds to trade with, 
         *  a string with the currency which will be used to sell in asks, 
         *  a string with the currency which will be used in bids, 
         *  how many entries it should analyze before it starts trading, 
         *  and how many entries it should keep track of at a given time */
        StandardDeviationStrategy(
            Wallet* walletPointer, 
            std::string askFundsCurrency,
            std::string bidFundsCurrency,
            unsigned int requiredNbrOfObservedEntriesToTrade, 
            unsigned int nbrOfObservedEntries
        );

        

        /** Clean pending orders which were fulfilled, or withdraw those which have still not been fulfilled since the last timestamp */
        std::list<OrderBookEntry> cleanPendingOrders(std::string currentTimestamp);
        
        /** Use the orders of the current timestamp to factor them into our metrics, 
         *  working out the mean, variance and standard deviations of the entire timeslice
         *  of entries that we are observing, determined by nbrOfEntriesToObserve */
        void updateMetrics(std::list<std::shared_ptr<OrderBookEntry>>& asks, std::list<std::shared_ptr<OrderBookEntry>>& bids);

        /** Use our metrics to determine whether we are at a point to place new bids or asks,
         *  and place those new orders into the listOfOrdersPlaced that's passed by reference */
        void tradeCurrentTimestamp(std::list<std::shared_ptr<OrderBookEntry>>& listOfOrdersPlaced);

        /** Track a given order to clean it up down the line through cleanPendingOrders */
        void trackOrder(std::shared_ptr<OrderBookEntry>& pointerToOrder);

    private:

        /** Our pointer to the wallet the strategy will use to trade */
        Wallet* wallet;

        /** The product this strategy is concerned with. Initialized in the constructor */
        std::string product;

        /** The name of the currency to ask and bid. These are the left-hand and right-hand currencies in a product */
        std::string askFundsCurrency;
        std::string bidFundsCurrency;

        /** A list of all the observed asks and bids that are used to keep our mean, variance and deviation metrics */
        std::list<std::shared_ptr<OrderBookEntry>> observedAsks;
        std::list<std::shared_ptr<OrderBookEntry>> observedBids;

        /** A list of the lowest asks and highest bids of each timestamp. These are what we use to trade against each timestamp */
        std::vector<std::shared_ptr<OrderBookEntry>> lowestAsks;
        std::vector<std::shared_ptr<OrderBookEntry>> highestBids;

        /** Lists of currently placed but unfulfilled orders */
        std::list<std::shared_ptr<OrderBookEntry>> pendingAsks;
        std::list<std::shared_ptr<OrderBookEntry>> pendingBids;
        
        /** Trading won't start until the size of the observed entries is at least this */
        unsigned int minimumNbrOfObservedEntriesToTrade = 0;

        /** The size of the observed entries will never be higher than this */
        unsigned int nbrOfEntriesToObserve = 0;

        /** Metrics to track for ask orders */
        double asksMean = 0;
        double asksVariance = 0;
        double asksOneStdDeviation = 0;
        double asksTwoStdDeviation = 0;

        /** Metrics to track for bid orders */
        double bidsMean = 0;
        double bidsVariance = 0;
        double bidsOneStdDeviation = 0;
        double bidsTwoStdDeviation = 0;

        /** Checks using our metrics and lists of lowest asks and highest bids 
         *  to determine whether it's a good time to place a bid or an ask */
        bool isBidSignal();
        bool isAskSignal();

        /** Place a bid or an ask. Checks that we have enough funds, and only
         *  tries to buy or sell a 20% of the best order that we found */
        void placeBid(std::list<std::shared_ptr<OrderBookEntry>>& listOfOrdersPlaced);
        void placeAsk(std::list<std::shared_ptr<OrderBookEntry>>& listOfOrdersPlaced);

        /** Calculate the mean of a list of orders */
        double getMean(std::list<std::shared_ptr<OrderBookEntry>>& elements);

        /** Calculate the variance of a list of orders */
        double getVariance(double mean, std::list<std::shared_ptr<OrderBookEntry>>& elements);
};