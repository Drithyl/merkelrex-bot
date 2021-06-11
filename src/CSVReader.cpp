
#include "../headers/CSVReader.h"
#include <iostream>
#include <fstream>

CSVReader::CSVReader()
{
    /** No need to initialize in the construction */
}

std::vector<OrderBookEntry> CSVReader::readCSV(std::string csvFilename)
{
    std::vector<OrderBookEntry> entries;

    std::ifstream csvFile{csvFilename};
    std::string line;

    if (csvFile.is_open())
    {
        while(std::getline(csvFile, line))
        {
            try
            {
                OrderBookEntry obe = stringsToOBE( tokenise(line, ',') );
                entries.push_back(obe);
            }

            catch(const std::exception& e)
            {
                std::cout << "CSVReader::readCSV read bad data." << std::endl;
            }
        }
    }

    std::cout << "CSVReader::readCSV read " << entries.size() << " entries." << std::endl;
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

OrderBookEntry CSVReader::stringsToOBE(std::vector<std::string> tokens)
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

    OrderBookEntry obe{
        price,
        amount,
        tokens[0],
        tokens[1],
        OrderBookEntry::stringToOrderBookType(tokens[2])
    };

    return obe;
}

OrderBookEntry CSVReader::stringsToOBE(
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

    OrderBookEntry obe{
        price,
        amount,
        timestamp,
        product,
        orderType
    };

    return obe;
}