
#pragma once

#include "OrderBookEntry.h"
#include <vector>
#include <list>
#include <string>
#include <memory>

class CSVReader
{
    public:
        CSVReader();

        /** Read a csv file and return a list of OrderBookEntry smart pointers */
        static std::list<std::shared_ptr<OrderBookEntry>> readCSV(std::string csvFile);

        /** Split a string into tokens using a given separator character */
        static std::vector<std::string> tokenise(std::string csvLine, char separator);

        /** Turn arguments into an OrderBookEntry object */
        static std::shared_ptr<OrderBookEntry> stringsToOBE(
            std::string price, 
            std::string amount, 
            std::string timestamp, 
            std::string product, 
            OrderBookType orderType
        );

    private:

        /** Turn a vector of strings into an OrderBookEntry object */
        static std::shared_ptr<OrderBookEntry> stringsToOBE(std::vector<std::string> strings);
};