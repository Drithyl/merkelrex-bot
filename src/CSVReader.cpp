
#include "../headers/CSVReader.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <map>

CSVReader::CSVReader()
{
    /** No need to initialize in the construction */
}

// Use shared smart pointers to the objects so we don't have to worry about freeing the memory manually
std::list<std::shared_ptr<OrderBookEntry>> CSVReader::readCSV(std::string csvFilename)
{
    auto start = std::chrono::system_clock::now();

    // Our list to store entries
    std::list<std::shared_ptr<OrderBookEntry>> entries;

    std::ifstream csvFile{csvFilename};
    std::string line;
    std::vector<std::string> tokenizedString;

    if (csvFile.is_open())
    {
        while(std::getline(csvFile, line))
        {
            try
            {
                // Create our entry and assign a pointer to it
                tokenizedString.clear();
                tokenizedString = tokenise(line, ',');
                std::shared_ptr<OrderBookEntry> entryPointer = stringsToOBE( tokenizedString );
                entries.push_back(entryPointer);
            }

            catch(const std::exception& e)
            {
                std::cout << "CSVReader::readCSV read bad data." << std::endl;
            }
        }

        csvFile.close();
    }

    // Record the current date time to compare it to the one before
    auto end = std::chrono::system_clock::now();

    // Get the elapsed seconds from when the bot started trading till now
    std::chrono::duration<double> elapsed_seconds = end-start;
    
    // Print the amount of time it took for the bot to trade
    std::cout << "CSVReader::readCSV read " << entries.size() << " entries in " << elapsed_seconds.count() << " seconds" << std::endl;

    return entries;
}

std::vector<std::string> CSVReader::tokenise(std::string csvLine, char separator)
{
    std::vector<std::string> tokens;
    signed int start, end;
    std::string token;

    /** find first char that is not our separator */
    start = csvLine.find_first_not_of(separator, 0);

    do
    {
        /** find next separator after start */
        end = csvLine.find_first_of(separator, start);

        /** reached end of line */
        if (start == csvLine.length() || start == end)
            break;

        /** if this is not last token, chop at end */
        if (end >= 0)
            token = csvLine.substr(start, end - start);

        /** if this is last token, chop at end of line */
        else 
            token = csvLine.substr(start, csvLine.length() - start);

        tokens.push_back(token);

        start = end + 1;
    } 
    
    /** iterate while there are tokens left at the end */
    while (end > 0);

    return tokens;
}

std::shared_ptr<OrderBookEntry> CSVReader::stringsToOBE(std::vector<std::string> tokens)
{
    double price, amount;

    /** this csv data file has five elements per line */
    if (tokens.size() != 5)
    {
        std::cout << "Bad line" << std::endl;
        throw std::exception{};
    }

    try
    {
        /** price and amount are in 3rd and 4th position, 
         * we use stod() to convert string to double */
        price = std::stod(tokens[3]);
        amount = std::stod(tokens[4]);
    }

    catch(const std::exception& e)
    {
        std::cout << "CSVReader::stringsToOBE Bad float! " << tokens[3] << " or " << tokens[4] << std::endl;
        throw e;
    }

    return std::make_shared<OrderBookEntry>(
        price,
        amount,
        tokens[0],
        tokens[1],
        OrderBookEntry::stringToOrderBookType(tokens[2])
    );
}

std::shared_ptr<OrderBookEntry> CSVReader::stringsToOBE(
    std::string priceString, 
    std::string amountString, 
    std::string timestamp, 
    std::string product, 
    OrderBookType orderType
)
{
    double price, amount;

    try
    {
        price = std::stod(priceString);
        amount = std::stod(amountString);
    }

    catch(const std::exception& e)
    {
        std::cout << "CSVReader::stringsToOBE Bad float! " << priceString << " or " << amountString << std::endl;
        throw e;
    }

    return std::make_shared<OrderBookEntry>(
        price,
        amount,
        timestamp,
        product,
        orderType
    );
}