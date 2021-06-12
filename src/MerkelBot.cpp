
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <list>
#include "../headers/PrecisionString.h"
#include "../headers/MerkelBot.h"

MerkelBot::MerkelBot(
    Wallet& walletGranted,
    std::vector<std::string> _productsToTrade
)
    // Deference the passed wallet reference and assign it to out wallet pointer
:   wallet(&walletGranted)
{

    // Grant some currency of every type to the bot to trade with
    wallet->insertCurrency("BTC", 0.1);
    wallet->insertCurrency("ETH", 10);
    wallet->insertCurrency("DOGE", 1000);
    wallet->insertCurrency("USDT", 100);

    // Log the initial wallet into our activity, so it can be compared with the wallet at the end of the simulation
    activityLog.insert(activityLog.begin(), "Initial wallet: \n\n" + wallet->toString() + "\n");

    // Loop through products that bot will trade and initialize new Strategy objects for each of them.
    // Right now we are only using a StandardDeviation Strategy, but an interface could be created so
    // the bot would be able to trade different products using different strategies
    for (std::string& product : _productsToTrade)
    {
        std::vector<std::string> currencies = CSVReader::tokenise(product, '/');
        StandardDeviationStrategy strategy{ wallet, currencies[0], currencies[1], 100, 300 };
        productStrategies.insert({ product, strategy });
        std::cout << "Bot will trade in " << product << std::endl;
    }
}

void MerkelBot::processNewTimestamp(OrderBook& orderBook, std::list<std::shared_ptr<OrderBookEntry>>& listOfOrdersPlaced)
{
    std::string currentTimestamp = orderBook.getCurrentTimestamp();

    // This **MUST** be a reference pair or it will create copies of the strategy items, so they'll never get updated
    for (auto& pair : productStrategies)
    {
        // Set aliases for our pair for easier understanding
        std::string product = pair.first;
        StandardDeviationStrategy& strategy = pair.second;

        // Clean pending orders and insert them into the withdrawnOrders list
        std::list<OrderBookEntry> withdrawnOrders = strategy.cleanPendingOrders(currentTimestamp);

        // Record all the withdrawn orders into our log
        for (OrderBookEntry& orderWithdrawn : withdrawnOrders)
        {
            recordWithdrawnOrder(orderWithdrawn);
        }
        
        // Get all the orders from the current timestamp in the orderbook
        std::list<std::shared_ptr<OrderBookEntry>> currentAsks = orderBook.getCurrentOrders(OrderBookType::ask, product);
        std::list<std::shared_ptr<OrderBookEntry>> currentBids = orderBook.getCurrentOrders(OrderBookType::bid, product);

        // Pass them to our strategy to update its metrics and then make the necessary trades, if any,
        // which will be inserted into the listOfOrdersPlaced that we are passing by reference
        strategy.updateMetrics(currentAsks, currentBids);
        strategy.tradeCurrentTimestamp(listOfOrdersPlaced);
    }
}

// Log withdrawn order
void MerkelBot::recordWithdrawnOrder(OrderBookEntry& order)
{
    std::string type;
    std::string timestamp = order.timestamp;

    // Separate currencies of the order
    std::vector<std::string> orderCurrencies = CSVReader::tokenise(order.product, '/');
    std::string offeredProduct;
    std::string requestedProduct;

    // Assign the correct types and products based on the type of order
    if (order.orderType == OrderBookType::ask)
    {
        type = "Ask";
        offeredProduct = orderCurrencies[0];
        requestedProduct = orderCurrencies[1];
    }

    else if (order.orderType == OrderBookType::bid)
    {
        type = "Bid";
        offeredProduct = orderCurrencies[1];
        requestedProduct = orderCurrencies[0];
    }

    else
    {
        type = "Unknown";
        offeredProduct = orderCurrencies[0];
        requestedProduct = orderCurrencies[1];
    }

    // Add the log entry as separate strings in a vector; each string meant to be separated by a tab character, as a "column"
    std::vector<std::string> logColumns = {timestamp, type + " withdrawn", order.product, "Offered: " + to_string_with_precision(order.amount, 10) + " " + offeredProduct, "Requested: " + requestedProduct, "at " + to_string_with_precision(order.price, 10)};
    addToLog(logColumns);
}

// Log an order placed by the bot
void MerkelBot::recordOrder(OrderBookEntry& order)
{
    std::string type;
    std::string timestamp = order.timestamp;

    // Separate currencies of the order
    std::vector<std::string> orderCurrencies = CSVReader::tokenise(order.product, '/');
    std::string offeredProduct;
    std::string requestedProduct;

    // Vector that will be used to log our string properly using the addToLog helper function
    std::vector<std::string> logColumns;

    // Properly set the type and products, and create a custom log entry relative to the type of order
    if (order.orderType == OrderBookType::ask)
    {
        type = "Ask";
        offeredProduct = orderCurrencies[0];
        requestedProduct = orderCurrencies[1];

        // Add the log entry as separate strings in a vector; each string meant to be separated by a tab character, as a "column"
        logColumns = {
            timestamp,
            type,
            order.product,
            "Offers: " + to_string_with_precision(order.amount, 10) + " " + offeredProduct,
            "Requests: " + to_string_with_precision(order.price, 10) + " " + requestedProduct + "/unit"
        };
    }

    else if (order.orderType == OrderBookType::bid)
    {
        type = "Bid";
        offeredProduct = orderCurrencies[1];
        requestedProduct = orderCurrencies[0];

        // Add the log entry as separate strings in a vector; each string meant to be separated by a tab character, as a "column"
        logColumns = {
            timestamp,
            type,
            order.product,
            "Offers: " + to_string_with_precision(order.price, 10) + " " + offeredProduct + "/unit",
            "Requests: " + to_string_with_precision(order.amount, 10) + " " + requestedProduct
        };
    }

    else
    {
        type = "Unknown";
        offeredProduct = orderCurrencies[0];
        requestedProduct = orderCurrencies[1];
    }

    addToLog(logColumns);
}

// Log a sale of one of the bot's orders
void MerkelBot::recordSale(OrderBookEntry& sale)
{
    std::string type;
    std::string timestamp = sale.timestamp;

    // Separate currencies of the order
    std::vector<std::string> saleCurrencies = CSVReader::tokenise(sale.product, '/');

    double outgoingAmount;
    std::string outgoingProduct;

    double incomingAmount;
    std::string incomingProduct;

    // Properly set the type and products, and create a custom log entry relative to the type of order
    if (sale.orderType == OrderBookType::asksale)
    {
        type = "Asksale";

        outgoingAmount = sale.amount;
        incomingAmount = sale.amount * sale.price;
        
        outgoingProduct = saleCurrencies[0];
        incomingProduct = saleCurrencies[1];
    }

    else if (sale.orderType == OrderBookType::bidsale)
    {
        type = "Bidsale";

        outgoingAmount = sale.amount * sale.price;
        incomingAmount = sale.amount;
        
        outgoingProduct = saleCurrencies[1];
        incomingProduct = saleCurrencies[0];
    }

    else
    {
        type = "Unknown";

        outgoingAmount = sale.amount;
        incomingAmount = sale.amount * sale.price;
        
        outgoingProduct = saleCurrencies[0];
        incomingProduct = saleCurrencies[1];
    }

    // Add the log entry as separate strings in a vector; each string meant to be separated by a tab character, as a "column"
    std::vector<std::string> logColumns = {
        timestamp,
        type,
        sale.product,
        "Paid " + to_string_with_precision(outgoingAmount, 10) + " " + outgoingProduct,
        "Received " + to_string_with_precision(incomingAmount, 10) + " " + incomingProduct
    };

    addToLog(logColumns);
}

// Iterate through our entire activity log and write every line into a log file in the specified path
void MerkelBot::writeLogToFile(std::string path)
{
    // Create stream to the file
    std::ofstream logFile(path + "bot_trade_log.txt");
    auto it = activityLog.begin();

    // Iterate through all the activityLog entries
    while (it != activityLog.end())
    {
        // Fetch the record at the front, stream it into the file and then erase the record from the list to continue iterating
        std::string record = activityLog.front();
        logFile << record + "\n";
        it = activityLog.erase(it);
    }

    // Add the state of the final wallet to compare it to what the bot started with
    logFile << "\n\nFinal wallet: \n\n" << wallet->toString() << "\n";

    // Close the file after we're done streaming
    logFile.close();
}

// Helper function to properly format new log entries. Each element in the received vector is a string
// which is meant to be separated from others by a tab character
void MerkelBot::addToLog(std::vector<std::string>& columns)
{
    std::string formattedStr = "";

    for (int i = 0; i < columns.size(); i++)
    {
        // When i = 1, we are logging the type of order, so no need to resize the strings to a large size
        if (i == 1)
            columns[i].resize(9, ' ');

        // When i = 2, we are logging the product of the order, so no need to resize the strings to a large size
        else if (i == 2)
            columns[i].resize(11, ' ');

        // Otherwise we are dealing with longer strings like the timestamp or what was offered and received, so we need a size of 27 characters between each tab
        else if (columns[i].size() < 27)
            columns[i].resize(27, ' ');

        // Put all the resized "column" strings together into a single string
        formattedStr += columns[i] + "\t";
    }

    // Push the newly formatted string into the activityLog
    activityLog.push_back(formattedStr);
}