

#pragma once

#include <map>
#include "Wallet.h"
#include "OrderBook.h"
#include "CSVReader.h"
#include "StandardDeviationStrategy.h"

class MerkelBot
{

    public:

        /** Constructor takes a reference to a wallet with which to trade, 
         *  and a vector of the products it will trade in. 
         *  This vector will be used to fill the productStrategies map */
        MerkelBot(Wallet& wallet, std::vector<std::string> _productsToTrade);

        /** Will fetch the orders from the current timestamp from the orderbook and pass them
         *  to the corresponding product strategy to update its metrics and potentially place
         *  new orders into the listOfOrdersPlaced */
        void processNewTimestamp(OrderBook& orderBook, std::list<std::shared_ptr<OrderBookEntry>>& listOfOrdersPlaced);
        
        /** Logs the placement of an order into the activityLog */
        void recordOrder(OrderBookEntry& order);

        /** Logs a sale into the activityLog */
        void recordSale(OrderBookEntry& sale);

        /** Dumps the entire activityLog into a text file so the trade history can be inspected */
        void writeLogToFile(std::string path);

    private:

        /** Pointer to the wallet that the bot receives. Must be a pointer so it stays updated no matter where it's modified, rather than copying it */
        Wallet* wallet;

        /** List of strings to be logged when writeLogToFile gets called */
        std::list<std::string> activityLog;

        /** Map of the Strategy objects that each product will follow; this gets filled on the bot constructor */
        std::map<std::string, StandardDeviationStrategy> productStrategies;

        /** Logs an order that was withdrawn into the activityLog */
        void recordWithdrawnOrder(OrderBookEntry& order);

        /** Helper function to add to the activityLog making sure that the tab spaces between each 
         * "column" in the string are respected, so the final log looks neat and well separated */
        void addToLog(std::vector<std::string>& columns);

};