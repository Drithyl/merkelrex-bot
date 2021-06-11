
/** forces compiler to include this file only once in each class definition; standard procedure */
#pragma once

#include <vector>
#include "../headers/OrderBookEntry.h"
#include "../headers/OrderBook.h"
#include "../headers/Wallet.h"

class MerkelMain
{
    public:

        MerkelMain();

        /** Call this to start the sim */
        void init();

    private:

        void printMenu();
        int getUserOption();
        void printHelp();
        void printMarketStats();
        void enterAsk();
        void enterBid();
        void printWallet();
        void goToNextTimeframe();
        void exitApp();
        void processOption(int userOption);

        std::string currentTime;

        //OrderBook orderBook{"../data/20200317.csv"};
        OrderBook orderBook{"../data/20200601.csv"};
        
        Wallet wallet;
};