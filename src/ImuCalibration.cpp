#include <iostream>
#include "Controller.h"

using namespace std;
using namespace Politocean;

int main(){
    Controller controller;

    controller.setup();
    
    int nReading = 0;
    int value = 0;
    int acc[3] = { 0, 0, 0 };

    while(true){
        value |= controller.SPIDataRW(0xFF);

        if(nReading % 2 == 1){
            acc[nReading / 2] = value;
            value = 0;
        }

        if(nReading == 5){
            cout << acc[0] << "\t" << acc[1] << "\t" << acc[2] << endl;
            nReading = 0;
        }else
            nReading++;
    }

    return 0;
}