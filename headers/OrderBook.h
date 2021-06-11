
#pragma once

#include "OrderBookEntry.h"
#include "CSVReader.h"
#include <string>
#include <vector>

class OrderBook
{
    public:
        /** construct, reading a csv data file */
        OrderBook(std::string filename);
        
        /** return vector of all known products in the dataset */
        std::vector<std::string> getKnownProducts();

        /** return vector of Orders according to the sent filters */
        std::vector<OrderBookEntry> getOrders(
            OrderBookType type, 
            std::string product, 
            std::string timestamp
        );

        /** return earliest timestamp. Assumes data is ordered from earliest to latest */
        std::string getEarliestTime();

        /** return timestamp after the one passed in; if there is no next one it will wrap around */
        std::string getNextTime(std::string timestamp);

        /** insert a new order into the OrderBook and sort it */
        void insertOrder(OrderBookEntry& order);
        
        /** match orders together and create sales */
        std::vector<OrderBookEntry> matchAsksToBids(std::string product, std::string timestamp);

        /** return highest price in a series of orders */
        static double getHighPrice(std::vector<OrderBookEntry>& orders);
        
        /** return lowest price in a series of orders */
        static double getLowPrice(std::vector<OrderBookEntry>& orders);


    private:

        std::vector<OrderBookEntry> orders;
};