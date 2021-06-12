
#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <ctime>
#include "../headers/MerkelMain.h"
#include "../headers/CSVReader.h"

MerkelMain::MerkelMain()
{

}

void MerkelMain::init()
{
    int input;
    currentTime = orderBook.getEarliestTime();

    std::cout << "Initialized wallet: \n\n" << wallet.toString() << std::endl;

    while(true)
    {
        printMenu();
        input = getUserOption();
        processOption(input);
    }
}

void MerkelMain::printMenu()
{
    std::cout << "1: Print help" << std::endl;

    std::cout << "2: Print exchange stats" <<  std::endl;

    std::cout << "3: Make an ask" << std::endl;

    std::cout << "4: Make a bid" << std::endl;

    std::cout << "5: Let Bot trade this timestamp" << std::endl;

    std::cout << "6: Let Bot take over the rest of the simulation" << std::endl;

    std::cout << "7: Print wallet" << std::endl;

    std::cout << "8: Continue" << std::endl;

    std::cout << "9: Exit" << std::endl;

    std::cout << "=========================" << std::endl;
    std::cout << "Current time is: " << currentTime << std::endl;

    std::cout << "Type in 1-9" << std::endl;
}

int MerkelMain::getUserOption()
{
    int userOption;
    std::string line;

    std::getline(std::cin, line);

    try
    {
        userOption = std::stoi(line);
    }
    catch(const std::exception& e)
    {
        //No actions needed; processOption() will handle it
    }

    std::cout << "You chose: " << userOption << std::endl;

    return userOption;
}

void MerkelMain::printHelp()
{
    std::cout << "Help - your aim is to make money. Analyse the market and make bids and offers." << std::endl;
}

void MerkelMain::printMarketStats()
{
    for (std::string const& product : orderBook.getKnownProducts())
    {
        std::cout << "Product: " << product << std::endl;
        std::list<std::shared_ptr<OrderBookEntry>> entries = orderBook.getCurrentOrders(OrderBookType::ask, product);

        std::cout << "Asks seen: " << entries.size() << std::endl;
        std::cout << "Max ask: " << OrderBook::getHighPrice(entries) << std::endl;
        std::cout << "Min ask: " << OrderBook::getLowPrice(entries) << std::endl;
    }
}

void MerkelMain::enterAsk()
{
    std::cout << "Make an ask - enter the amount: product, price, amount, e.g. ETH/BTC, 200, 0.5" << std::endl;
    std::string input;

    std::getline(std::cin, input);

    std::vector<std::string> tokens = CSVReader::tokenise(input, ',');

    if (tokens.size() != 3)
        std::cout << "MerkelMain::enterAsk Bad input! " << input << std::endl;

    else
    {
        try
        {
            std::vector<std::string> currencies = CSVReader::tokenise(tokens[0], '/');
            std::shared_ptr<OrderBookEntry> obePointer = CSVReader::stringsToOBE(
                tokens[1],
                tokens[2],
                currentTime,
                tokens[0],
                OrderBookType::ask
            );
            
            obePointer->username = "simuser";

            if (wallet.canFulfillOrder(*obePointer))
            {
                std::cout << "Wallet looks good. " << std::endl;
                orderBook.insertOrder(obePointer);
                wallet.lockCurrency(currencies[0], obePointer->amount);
            }

            else std::cout << "Insufficient funds. " << std::endl;
        }

        catch(const std::exception& e)
        {
            std::cout << "MerkelMain::enterAsk Bad input! " << input << std::endl;
        }
    }
}

void MerkelMain::enterBid()
{
    std::cout << "Make a bid - enter the amount: product, price, amount, e.g. ETH/BTC, 200, 0.5" << std::endl;
    std::string input;

    std::getline(std::cin, input);

    std::vector<std::string> tokens = CSVReader::tokenise(input, ',');

    if (tokens.size() != 3)
        std::cout << "MerkelMain::enterBid Bad input! " << input << std::endl;

    else
    {
        std::cout << "MerkelMain::enterBid Got the input, converting... " << std::endl;

        try
        {
            std::vector<std::string> currencies = CSVReader::tokenise(tokens[0], '/');
            std::shared_ptr<OrderBookEntry> obePointer = CSVReader::stringsToOBE(
                tokens[1],
                tokens[2],
                currentTime,
                tokens[0],
                OrderBookType::bid
            );

            std::cout << "MerkelMain::enterBid Converted input, checking wallet... " << std::endl;

            obePointer->username = "simuser";

            if (wallet.canFulfillOrder(*obePointer))
            {
                std::cout << "Wallet looks good. Inserting order..." << std::endl;
                orderBook.insertOrder(obePointer);
                wallet.lockCurrency(currencies[1], obePointer->amount * obePointer->price);
                std::cout << "Order was inserted." << std::endl;
            }

            else std::cout << "Insufficient funds. " << std::endl;
        }

        catch(const std::exception& e)
        {
            std::cout << "MerkelMain::enterBid Bad input! " << input << std::endl;
        }
    }
}

// Let bot analyze and place trades for the current timestamp
void MerkelMain::letBotTrade()
{
    // Initialize an empty list that will be filled by the function below
    std::list<std::shared_ptr<OrderBookEntry>> listOfOrdersPlaced;

    // Pass the orderbook for the bot to process the current orders and place
    // newly created orders into the listOfOrdersPlaced
    bot.processNewTimestamp(orderBook, listOfOrdersPlaced);

    // Iterate through bot created orders and record them
    for (std::shared_ptr<OrderBookEntry> orderPlaced : listOfOrdersPlaced)
    {
        orderBook.insertOrder(orderPlaced);
        bot.recordOrder(*orderPlaced);
    }
}

// Bot will trade in an automated fashion throughout the entirety of the
// remaining timestamps until it reaches the end, at which point it will
// create a log file in the provided path below
void MerkelMain::letBotTakeOver()
{
    // Record the current date time for performance measuring
    auto start = std::chrono::system_clock::now();
    std::string firstTimestamp = orderBook.getEarliestTime();

    std::cout << "Bot begins to trade..." << std::endl;

    // Trade current timestamp and move to the next until we reach the first one again
    do
    {
        this->letBotTrade();
        this->goToNextTimestamp();

    } while (currentTime > firstTimestamp);

    // Dump the log into the given path
    bot.writeLogToFile("./logs/");

    // Record the current date time to compare it to the one before
    auto end = std::chrono::system_clock::now();

    // Get the elapsed seconds from when the bot started trading till now
    std::chrono::duration<double> elapsed_seconds = end-start;
    
    // Print the amount of time it took for the bot to trade
    std::cout << "Finished bot trading in " << elapsed_seconds.count() << " seconds. Total sales matched: " << totalSales << std::endl;
}

void MerkelMain::printWallet()
{
    std::cout << wallet.toString() << std::endl;
}

// Advance to the next timestamp, first matching all the current orders of each product and
// recording all the new sales, as well as updating the currencies in the wallet and logging them
void MerkelMain::goToNextTimestamp()
{
    auto start = std::chrono::system_clock::now();

    for (std::string& product : orderBook.getKnownProducts())
    {
        // To match orders current and past
        std::list<OrderBookEntry> sales = orderBook.matchCurrentAndPreviousOrders(product);

        // To match orders only within the same timestamp, not considering past ones
        //std::list<OrderBookEntry> sales = orderBook.matchOnlyCurrentOrders(product);

        for (OrderBookEntry& sale : sales)
        {
            totalSales++;

            if (sale.username != "dataset")
            {
                //update wallet
                wallet.processSale(sale);
                bot.recordSale(sale);
            }
        }
    }

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Timestamp " << currentTime << " (" << timestampIndex << ") completed in " << elapsed_seconds.count() << " seconds." << std:: endl;
    
    timestampIndex++;
    currentTime = orderBook.goToNextTimestamp();
}

void MerkelMain::exitApp()
{
    std::cout << "Exitting..." << std::endl;
    exit(0);
}

void MerkelMain::processOption(int userOption)
{
    //bad input
    if (userOption == 0)
    {
        std::cout << "Invalid choice. Choose 1-6." << std::endl;
    }

    else if (userOption == 1)
    {
        printHelp();
    }

    else if (userOption == 2)
    {
        printMarketStats();
    }

    else if (userOption == 3)
    {
        enterAsk();
    }

    else if (userOption == 4)
    {
        enterBid();
    }

    else if (userOption == 5)
    {
        letBotTrade();
        this->goToNextTimestamp();
    }

    else if (userOption == 6)
    {
        letBotTakeOver();
    }

    else if (userOption == 7)
    {
        printWallet();
    }

    else if (userOption == 8)
    {
        goToNextTimestamp();
    }

    else if (userOption == 9)
    {
        exitApp();
    }
}