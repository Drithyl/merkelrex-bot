
#include <iostream>
#include <vector>
#include "../headers/MerkelMain.h"
#include "../headers/CSVReader.h"

MerkelMain::MerkelMain()
{

}

void MerkelMain::init()
{
    int input;
    currentTime = orderBook.getEarliestTime();

    wallet.insertCurrency("BTC", 10);

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

    std::cout << "5: Print wallet" << std::endl;

    std::cout << "6: Continue" << std::endl;

    std::cout << "7: Exit" << std::endl;

    std::cout << "=========================" << std::endl;
    std::cout << "Current time is: " << currentTime << std::endl;

    std::cout << "Type in 1-7" << std::endl;
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
        std::vector<OrderBookEntry> entries = orderBook.getOrders(OrderBookType::ask, product, currentTime);

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
            OrderBookEntry obe = CSVReader::stringsToOBE(
                tokens[1],
                tokens[2],
                currentTime,
                tokens[0],
                OrderBookType::ask
            );

            obe.username = "simuser";

            if (wallet.canFulfillOrder(obe))
            {
                std::cout << "Wallet looks good. " << std::endl;
                orderBook.insertOrder(obe);
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
        try
        {
            OrderBookEntry obe = CSVReader::stringsToOBE(
                tokens[1],
                tokens[2],
                currentTime,
                tokens[0],
                OrderBookType::bid
            );

            obe.username = "simuser";

            if (wallet.canFulfillOrder(obe))
            {
                std::cout << "Wallet looks good. " << std::endl;
                orderBook.insertOrder(obe);
            }

            else std::cout << "Insufficient funds. " << std::endl;
        }

        catch(const std::exception& e)
        {
            std::cout << "MerkelMain::enterBid Bad input! " << input << std::endl;
        }
    }
}

void MerkelMain::printWallet()
{
    std::cout << wallet.toString() << std::endl;
}

void MerkelMain::goToNextTimeframe()
{
    std::cout << "Going to next time frame." << std::endl;

    for (std::string& product : orderBook.getKnownProducts())
    {
        std::cout << "Matching " << product << std::endl;
        std::vector<OrderBookEntry> sales = orderBook.matchAsksToBids(product, currentTime);
        std::cout << "Sales: " << sales.size() << std::endl;

        for (OrderBookEntry& sale : sales)
        {
            std::cout << "Sale price: " << sale.price << " amount: " << sale.amount << std::endl;

            if (sale.username != "dataset")
            {
                //update wallet
                wallet.processSale(sale);
            } 
        }
    }

    currentTime = orderBook.getNextTime(currentTime);
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
        printWallet();
    }

    else if (userOption == 6)
    {
        goToNextTimeframe();
    }

    else if (userOption == 7)
    {
        exitApp();
    }
}