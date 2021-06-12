
#include "../headers/OrderBookEntry.h"
#include "../headers/MerkelMain.h"
#include "../headers/CSVReader.h"
#include "../headers/Wallet.h"

/*  To compile: 
    g++ --std=c++11 ./src/*.cpp -o merkel.exe -O2
*/

int main()
{
    MerkelMain app{};
    app.init();
}