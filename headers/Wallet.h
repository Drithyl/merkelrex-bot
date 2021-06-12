
#pragma once

#include <string>
#include <map>
#include <list>
#include "OrderBookEntry.h"

class Wallet
{
    public:

        Wallet();

        /** insert currency to the wallet */
        void insertCurrency(std::string type, double amount);

        /** remove currency from the wallet */
        bool removeCurrency(std::string type, double amount);

        /** check if the wallet contains this much funds or more */
        bool containsCurrency(std::string type, double amount);
        
        /** lock a given amount of funds into lockedCurrencies, removing it from currencies,
         *  so it does not get used into a different trade while the order has not been fulfilled */
        void lockCurrency(std::string type, double amount);

        /** unlock a given amount of funds from lockedCurrencies, adding it from currencies.
         *  this will be used whenever an order is withdrawn */
        void unlockCurrency(std::string type, double amount);

        /** return the amount of funds of this currency */
        double getCurrentFunds(std::string type);

        /** check if the wallet can cope with this ask or bid */
        bool canFulfillOrder(OrderBookEntry& order);

        /** adds or takes funds resulting from a sale, and assumes it was made by the owner of the wallet */
        void processSale(OrderBookEntry& sale);

        /** print the contents of the wallet in a string representation */
        std::string toString();
        

    private:

        /** a map of our currencies available */
        std::map<std::string, double> currencies;

        /** a map of the currencies we have locked in into placed orders, but not fulfilled */
        std::map<std::string, double> lockedCurrencies;
};