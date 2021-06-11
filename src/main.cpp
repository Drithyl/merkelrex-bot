
#include "../headers/OrderBookEntry.h"
#include "../headers/MerkelMain.h"
#include "../headers/CSVReader.h"
#include "../headers/Wallet.h"

/*  To compile, cd to src and then: 
    g++ --std=c++11 main.cpp *.cpp
*/

int main()
{
    MerkelMain app{};
    app.init();
    

    /*
    Wallet wallet;
    wallet.insertCurrency("BTC", 10000);

    std::cout << "Wallet has BTC " << wallet.containsCurrency("BTC", 10) << std::endl;
    wallet.removeCurrency("BTC", 1000);
    std::cout << wallet.toString() << std::endl;
    */

}