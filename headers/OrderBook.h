
#pragma once

#include "OrderBookEntry.h"
#include "CSVReader.h"
#include <string>
#include <vector>
#include <memory>
#include <list>
#include <map>

class OrderBook
{
    public:
        /** constructor, reading a csv data file */
        OrderBook(std::string filename);
        
        /** return vector of all known products in the dataset */
        std::vector<std::string> getKnownProducts();

        /** return list of Orders according to the sent filters in the current timestamp */
        std::list<std::shared_ptr<OrderBookEntry>> getCurrentOrders(
            OrderBookType type, 
            std::string product
        );

        /** return earliest timestamp. Assumes data is ordered from earliest to latest */
        std::string getEarliestTime();

        /** return current timestamp. */
        std::string getCurrentTimestamp();

        /** advance the indexes of the orders list to the ones of the next timestamp */
        std::string goToNextTimestamp();

        /** insert a new order into the OrderBook and sort it */
        std::shared_ptr<OrderBookEntry>& insertOrder(std::shared_ptr<OrderBookEntry>& order);

        /** match only current orders together and create sales */
        std::list<OrderBookEntry> matchOnlyCurrentOrders(std::string product);
        
        /** match current and past orders together and create sales */
        std::list<OrderBookEntry> matchCurrentAndPreviousOrders(std::string product);

        /** return highest price in a series of orders */
        static double getHighPrice(std::list<std::shared_ptr<OrderBookEntry>>& orders);
        
        /** return lowest price in a series of orders */
        static double getLowPrice(std::list<std::shared_ptr<OrderBookEntry>>& orders);

        /** return lowest priced entry in a series of orders */
        static std::shared_ptr<OrderBookEntry> getLowPriceEntry(std::list<std::shared_ptr<OrderBookEntry>>& orders);

        /** return highest priced entry in a series of orders */
        static std::shared_ptr<OrderBookEntry> getHighPriceEntry(std::list<std::shared_ptr<OrderBookEntry>>& orders);


    private:

        std::list<std::shared_ptr<OrderBookEntry>> orders;
        std::vector<std::string> knownProducts;

        /** keep a growing map of asks and bids that have been iterated through */
        std::map<std::string, std::list<std::shared_ptr<OrderBookEntry>>> mapOfAsksTillCurrent;
        std::map<std::string, std::list<std::shared_ptr<OrderBookEntry>>> mapOfBidsTillCurrent;

        /** the current timestamp */
        std::string currentTime;
        
        /** a vector of iterators; each of them pointing to the start of a new timestamp in our orders */
        std::vector<std::list<std::shared_ptr<OrderBookEntry>>::iterator> timestampIterators;

        /** the current position in our vector of iterators timestampIterators */
        int currentPositionInTimestampIterators = 0;

        /** advance the index that marks the current position in the timestampIterators array; wrapping to 0 if we're at the end */
        void goToNextTimestampIndex();

        /** work out the index of the last order within the current timestamp based on the index of the first order */
        std::list<std::shared_ptr<OrderBookEntry>>::iterator getEndItOfCurrentTimestamp();

        /** check if an ask matches a bid, in which case, it inserts a sale entry into the matches list */
        void matchAskToBid(std::shared_ptr<OrderBookEntry>& ask, std::shared_ptr<OrderBookEntry>& bid, std::list<OrderBookEntry>& matches);
        
        /** check if an ask matches a bid, in which case, it inserts a sale entry into the matches list */
        void matchAsksToBids(std::list<std::shared_ptr<OrderBookEntry>>& asks, std::list<std::shared_ptr<OrderBookEntry>>& bids, std::list<OrderBookEntry>& matches);
        
        /** insert into an already sorted list following the same sorting function provided. This assumes that the newEntries list given is also sorted by the same function beforehand! */
        void insertIntoSortedList(std::list<std::shared_ptr<OrderBookEntry>>& newEntries, std::list<std::shared_ptr<OrderBookEntry>>& sortedList, bool (*sortFunction)(const std::shared_ptr<OrderBookEntry>&, const std::shared_ptr<OrderBookEntry>&));

        /** define a template with a generic iterator type and a generic type contained in the list in question */
        template<typename genericIterator, typename pointerToListElementType>
        /** searches the iterator at the position before which an element should be inserted to be sorted according to the compFunction */
        genericIterator binarySearchInsertPosition(genericIterator begin, genericIterator end, pointerToListElementType elementToInsert, bool (*compFunction)(const pointerToListElementType&, const pointerToListElementType&));
};