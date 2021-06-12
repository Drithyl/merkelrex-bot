

#pragma once
#include <vector>
#include "Wallet.h"
#include "OrderBook.h"


class TradingStrategy
{
    public:

        TradingStrategy(Wallet wallet);

        void processNewPricePoint(double newPricePoint);

    protected:

        Wallet wallet;

};