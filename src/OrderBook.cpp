
#include "../headers/OrderBook.h"
#include "../headers/CSVReader.h"
#include <map>
#include <algorithm>

/** construct, reading a csv data file */
OrderBook::OrderBook(std::string filename)
{
    orders = CSVReader::readCSV(filename);
}

/** return vector of all known products in the dataset */
std::vector<std::string> OrderBook::getKnownProducts()
{
    std::vector<std::string> products;
    std::map<std::string, bool> productMap;

    /** initialize map entries with all product types */
    for (OrderBookEntry& entry : orders)
        productMap[entry.product] = true;

    /** 'auto' lets the compiler deal with the type rather than specifying one */
    for (auto const& entry : productMap)
    {
        /** entry.first is the map key, and entry.last is the actual value of the entry, a boolean */
        products.push_back(entry.first);
    }

    return products;
}

/** return vector of Orders according to the sent filters */
std::vector<OrderBookEntry> OrderBook::getOrders(
    OrderBookType type, 
    std::string product, 
    std::string timestamp
)
{
    std::vector<OrderBookEntry> orders_sub;

    for (OrderBookEntry& entry : orders)
    {
        if (entry.orderType == type && 
            entry.product == product && 
            entry.timestamp == timestamp)
        {
            orders_sub.push_back(entry);
        }
    }

    return orders_sub;
}

std::string OrderBook::getEarliestTime()
{
    return orders[0].timestamp;
}

std::string OrderBook::getNextTime(std::string timestamp)
{
    std::string next_timestamp = "";

    for (OrderBookEntry& entry : orders)
    {
        if (entry.timestamp > timestamp)
        {
            next_timestamp = entry.timestamp;
            break;
        }
    }

    /** wrap around in the data back to the earliest timestamp if we're at the end */
    if (next_timestamp == "")
        next_timestamp = OrderBook::getEarliestTime();

    return next_timestamp;
}

void OrderBook::insertOrder(OrderBookEntry& order)
{
    orders.push_back(order);
    std::sort(orders.begin(), orders.end(), OrderBookEntry::compareByTimestamp);
}

std::vector<OrderBookEntry> OrderBook::matchAsksToBids(std::string product, std::string timestamp)
{
    std::vector<OrderBookEntry> asks = getOrders(OrderBookType::ask, product, timestamp);
    std::vector<OrderBookEntry> bids = getOrders(OrderBookType::bid, product, timestamp);
    std::vector<OrderBookEntry> sales;

    /** lowest bids (sells) will be matched to highest asks (buys) in priority */
    std::sort(asks.begin(), asks.end(), OrderBookEntry::compareByPriceAsc);
    std::sort(bids.begin(), bids.end(), OrderBookEntry::compareByPriceDesc);

    for (OrderBookEntry& ask : asks)
    {
        for (OrderBookEntry& bid : bids)
        {
            if (bid.price >= ask.price)
            {
                OrderBookEntry sale{ask.price, 0, timestamp, product, OrderBookType::asksale};

                /** the user is placing a bid against the dataset, thus this will result in a bidsale */
                if (bid.username != "dataset")
                {
                    sale.username = "simuser";
                    sale.orderType = OrderBookType::bidsale;
                }

                /** the user is placing an ask against the dataset, thus this will result in an asksale */
                else if (ask.username != "dataset")
                {
                    sale.username = "simuser";
                    sale.orderType = OrderBookType::asksale;
                }

                /** both bid and ask are fully matched; both get wiped */
                if (bid.amount == ask.amount)
                {
                    sale.amount = ask.amount;
                    sales.push_back(sale);

                    bid.amount = 0;
                    ask.amount = 0;
                    break;
                }

                /** more product being bought than sold; bid gets sliced by the ask amount, and ask gets wiped */
                if (bid.amount > ask.amount)
                {
                    sale.amount = ask.amount;
                    sales.push_back(sale);

                    bid.amount -= ask.amount;
                    ask.amount = 0;
                    break;
                }

                /** more product being sold than bought; bid gets wiped and ask gets sliced by the bid amount */
                if (bid.amount < ask.amount && bid.amount > 0)
                {
                    sale.amount = bid.amount;
                    sales.push_back(sale);

                    ask.amount -= bid.amount;
                    bid.amount = 0;
                    continue;
                }
            }
        }
    }

    return sales;
}

double OrderBook::getHighPrice(std::vector<OrderBookEntry>& orders)
{
    double max = orders[0].price;

    for (OrderBookEntry& entry : orders)
        if (entry.price > max)
            max = entry.price;

    return max;
}

double OrderBook::getLowPrice(std::vector<OrderBookEntry>& orders)
{
    double min = orders[0].price;

    for (OrderBookEntry& entry : orders)
        if (entry.price < min)
            min = entry.price;

    return min;
}