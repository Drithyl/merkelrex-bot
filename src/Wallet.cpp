
#include "../headers/Wallet.h"
#include "../headers/CSVReader.h"

Wallet::Wallet()
{

}


void Wallet::insertCurrency(std::string type, double amount)
{
    double balance;

    if (amount < 0)
        throw std::exception{};

    /** check if the type of currency exists in the map */
    if (currencies.count(type) == 0)
        balance = 0;

    else balance = currencies[type];

    balance += amount;

    currencies[type] = balance;
}

bool Wallet::removeCurrency(std::string type, double amount)
{
    if (amount < 0)
        return false;

    /** check if the type of currency exists in the map */
    if (currencies.count(type) == 0)
        return false;

    /** there is enough currency to remove */
    else if (containsCurrency(type, amount))
    {
        currencies[type] -= amount;
        return true;
    }

    /** there is currency type, but not enough */
    else return false;
}

bool Wallet::containsCurrency(std::string type, double amount)
{
    if (currencies.count(type) == 0)
        return false;

    else return currencies[type] >= amount;
}

bool Wallet::canFulfillOrder(OrderBookEntry order)
{
    std::vector<std::string> currencies = CSVReader::tokenise(order.product, '/');

    /** ask */
    if (order.orderType == OrderBookType::ask)
    {
        /** in order to deliver this ask we need enough amount of the currency */
        double amount = order.amount;
        std::string currency = currencies[0];
        std::cout << "Wallet::canFulfillOrder " << currency << " : " << amount << std::endl;
        return containsCurrency(currency, amount);
    }

    /** bid */
    if (order.orderType == OrderBookType::bid)
    {
        /** in order to pay this bid we need the requested the amount to buy times the price at which it's sold */
        double amount = order.amount * order.price;
        std::string currency = currencies[1];
        std::cout << "Wallet::canFulfillOrder " << currency << " : " << amount << std::endl;
        return containsCurrency(currency, amount);
    }

    return false;
}

void Wallet::processSale(OrderBookEntry sale)
{
    std::vector<std::string> saleCurrencies = CSVReader::tokenise(sale.product, '/');

    /** ask */
    if (sale.orderType == OrderBookType::asksale)
    {
        /** user sold currency; outgoing is the amount we set, incoming is that amount times price we got for it */
        double outgoingAmount = sale.amount;
        double incomingAmount = sale.amount * sale.price;

        std::string outgoingCurrency = saleCurrencies[0];
        std::string incomingCurrency = saleCurrencies[1];

        currencies[incomingCurrency] += incomingAmount;
        currencies[outgoingCurrency] -= outgoingAmount;
    }

    /** bid */
    if (sale.orderType == OrderBookType::bidsale)
    {
        /** user bought currency; outgoing is the amount we set times price we paid, incoming is the amount we requested */
        double incomingAmount = sale.amount;
        double outgoingAmount = sale.amount * sale.price;

        std::string incomingCurrency = saleCurrencies[0];
        std::string outgoingCurrency = saleCurrencies[1];

        currencies[incomingCurrency] += incomingAmount;
        currencies[outgoingCurrency] -= outgoingAmount;
    }
}

std::string Wallet::toString()
{
    std::string walletStr;

    for (std::pair<std::string, double> pair : currencies)
    {
        std::string currency = pair.first;
        double amount = pair.second;

        walletStr += currency + ": " + std::to_string(amount) + "\n";
    }

    return walletStr;
}