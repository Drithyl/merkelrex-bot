
#include "../headers/PrecisionString.h"
#include "../headers/Wallet.h"
#include "../headers/CSVReader.h"

Wallet::Wallet()
{

}


void Wallet::insertCurrency(std::string type, double amount)
{
    double balance;

    if (amount < 0.000000001f)
        throw std::exception{};

    // Check if the type of currency exists in the map
    if (currencies.count(type) == 0)
        balance = 0;

    else balance = currencies[type];

    balance += amount;

    currencies[type] = balance;
}

bool Wallet::removeCurrency(std::string type, double amount)
{
    if (amount < 0.000000001f)
        return false;

    // Check if the type of currency exists in the map
    if (currencies.count(type) == 0)
        return false;

    // There is enough currency to remove
    else if (containsCurrency(type, amount))
    {
        currencies[type] -= amount;
        return true;
    }

    // There is currency type, but not enough
    else return false;
}

// Check if wallet contains a given amount of currency
bool Wallet::containsCurrency(std::string type, double amount)
{
    if (currencies.count(type) == 0)
        return false;

    else return currencies[type] >= amount;
}

// Lock away some funds so they do not get invested while they are already used in a pending order
void Wallet::lockCurrency(std::string type, double amount)
{
    // If no funds of this type exist in our wallet, ignore
    if (currencies.count(type) == 0)
        return;

    if (currencies[type] < amount - 0.000001f)
    {
        std::cout << "Exception: Tried to lock " << to_string_with_precision(amount, 20) << " " << type << ", have " << to_string_with_precision(currencies[type], 20) << std::endl;
        throw std::exception{};
    }
        
    // Check if the type of currency exists in the map and initialize it if not
    if (lockedCurrencies.count(type) == 0)
        lockedCurrencies[type] = 0;

    currencies[type] -= amount;
    lockedCurrencies[type] += amount;
}

// Unlock some funds so can get invested again if the order was withdrawn
void Wallet::unlockCurrency(std::string type, double amount)
{
    // If no funds of this type exist in our locked currencies, ignore
    if (lockedCurrencies.count(type) == 0)
        return;
        
    // Check if the type of currency exists in the map and initialize it if not
    if (currencies.count(type) == 0)
        currencies[type] = 0;
    
    // If there aren't enough funds locked as requested, throw an exception
    if (lockedCurrencies[type] < amount - 0.000001f)
    {
        std::cout << "Exception: Tried to unlock " << to_string_with_precision(amount, 20) << " " << type << ", have " << to_string_with_precision(lockedCurrencies[type], 20) << std::endl;
        throw std::exception{};
    }

    lockedCurrencies[type] -= amount;
    currencies[type] += amount;
}

// Get the amount of funds for a given type
double Wallet::getCurrentFunds(std::string type)
{
    if (currencies.count(type) == 0)
        return 0;

    return currencies[type];
}

bool Wallet::canFulfillOrder(OrderBookEntry& order)
{
    std::vector<std::string> currencies = CSVReader::tokenise(order.product, '/');

    if (order.orderType == OrderBookType::ask)
    {
        // in order to deliver this ask we need enough amount of the currency
        double amount = order.amount;
        std::string currency = currencies[0];
        std::cout << "Wallet::canFulfillOrder " << currency << " : " << amount << std::endl;
        return containsCurrency(currency, amount);
    }

    if (order.orderType == OrderBookType::bid)
    {
        // in order to pay this bid we need the requested the amount to buy times the price at which it's sold
        double amount = order.amount * order.price;
        std::string currency = currencies[1];
        std::cout << "Wallet::canFulfillOrder " << currency << " : " << amount << std::endl;
        return containsCurrency(currency, amount);
    }

    std::cout << "Wallet cannot fulfill order." << std::endl;
    return false;
}

void Wallet::processSale(OrderBookEntry& sale)
{
    std::vector<std::string> saleCurrencies = CSVReader::tokenise(sale.product, '/');

    if (sale.orderType == OrderBookType::asksale)
    {
        // user sold currency; outgoing is the amount we set, incoming is that amount times price we got for it
        double outgoingAmount = sale.amount;
        double incomingAmount = sale.amount * sale.price;

        std::string outgoingCurrency = saleCurrencies[0];
        std::string incomingCurrency = saleCurrencies[1];

        currencies[incomingCurrency] += incomingAmount;
        lockedCurrencies[outgoingCurrency] -= outgoingAmount;
    }

    if (sale.orderType == OrderBookType::bidsale)
    {
        // user bought currency; outgoing is the amount we set times price we paid, incoming is the amount we requested
        double incomingAmount = sale.amount;
        double outgoingAmount = sale.amount * sale.price;

        std::string incomingCurrency = saleCurrencies[0];
        std::string outgoingCurrency = saleCurrencies[1];

        currencies[incomingCurrency] += incomingAmount;
        lockedCurrencies[outgoingCurrency] -= outgoingAmount;
    }
}

// Print the contents of the wallet
std::string Wallet::toString()
{
    std::string walletStr;

    for (auto& pair : currencies)
    {
        std::string currency = pair.first;
        double amount = pair.second;

        if (lockedCurrencies.count(currency) == 1)
            amount += lockedCurrencies[currency];

        walletStr += currency + ": " + to_string_with_precision(amount, 10) + "\n";
    }

    return walletStr;
}