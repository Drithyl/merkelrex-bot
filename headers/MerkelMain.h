
/** forces compiler to include this file only once in each class definition; standard procedure */
#pragma once

#include <list>
#include <memory>
#include "../headers/OrderBookEntry.h"
#include "../headers/OrderBook.h"
#include "../headers/Wallet.h"
#include "../headers/MerkelBot.h"

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
        void letBotTrade();
        void letBotTakeOver();
        void printWallet();
        void goToNextTimestamp();
        void exitApp();
        void processOption(int userOption);

        std::string currentTime;

        OrderBook orderBook{"G:\\Coursera\\Bachelor of Science in Computer Science\\3rd Semester\\(CM2005) Object-Oriented Programming\\Projects\\Midterm - Merklerex bot - Pointers\\data\\20200601.csv"};
        
        Wallet wallet;
        MerkelBot bot{wallet, orderBook.getKnownProducts()};

        unsigned int totalSales = 0;
        unsigned int timestampIndex = 0;
};