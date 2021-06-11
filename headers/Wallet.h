
#pragma once

#include <string>
#include <map>
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

        /** check if the wallet can cope with this ask or bid */
        bool canFulfillOrder(OrderBookEntry order);

        /** adds or takes funds resulting from a sale, and assumes it was made by the owner of the wallet */
        void processSale(OrderBookEntry sale);

        /** print the contents of the wallet in a string representation */
        std::string toString();
        

    private:

        std::map<std::string, double> currencies;
};