
#include "../headers/OrderBookEntry.h"

/** OrderBookEntry is the class namespace, then ::OrderBookEntry is the
 *  function we are accessing; in our case, the constructor, to define
 *  its implementation
 */
OrderBookEntry::OrderBookEntry(
    double _price, 
    double _amount, 
    std::string _timestamp, 
    std::string _product, 
    OrderBookType _orderType,
    std::string _username
)
/** INIT list; preferred way to initialize constructor variables
 *  Is also more efficient since it does not create any copies of values 
 */
:   price(_price), 
    amount(_amount), 
    timestamp(_timestamp), 
    product(_product), 
    orderType(_orderType), 
    username(_username)
{

}

OrderBookType OrderBookEntry::stringToOrderBookType(std::string s)
{
    if (s == "ask")
        return OrderBookType::ask;

    if (s == "bid")
        return OrderBookType::bid;

    else return OrderBookType::unknown;
}

bool OrderBookEntry::compareByTimestamp(OrderBookEntry& e1, OrderBookEntry& e2)
{
    return e1.timestamp < e2.timestamp;
}

bool OrderBookEntry::compareByPriceAsc(OrderBookEntry& e1, OrderBookEntry& e2)
{
    return e1.price < e2.price;
}

bool OrderBookEntry::compareByPriceDesc(OrderBookEntry& e1, OrderBookEntry& e2)
{
    return e1.price > e2.price;
}
