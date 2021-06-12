
/** forces compiler to include this file only once in each .cpp; standard procedure */
#pragma once

#include <iostream>
#include <memory>

/** unknown type added for strings that don't conform; should throw exception instead */
enum class OrderBookType{bid, ask, bidsale, asksale, unknown};

/** Class specification without implementation
 */
class OrderBookEntry
{
    public:

        OrderBookEntry( double _price, 
                        double _amount, 
                        std::string _timestamp, 
                        std::string _product, 
                        OrderBookType _orderType,
                        std::string username = "dataset" ); //default to dataset for all the orders from the csv data

        static OrderBookType stringToOrderBookType(std::string s);
        static bool compareByTimestamp(const OrderBookEntry& e1, const OrderBookEntry& e2);
        static bool compareByPriceAsc(const std::shared_ptr<OrderBookEntry>& e1, const std::shared_ptr<OrderBookEntry>& e2);
        static bool compareByPriceDesc(const std::shared_ptr<OrderBookEntry>& e1, const std::shared_ptr<OrderBookEntry>& e2);

        double price;
        double amount;
        std::string timestamp;
        std::string product;
        OrderBookType orderType;
        std::string username;
};