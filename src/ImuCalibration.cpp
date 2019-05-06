#include <iostream>
#include "Controller.h"
#include "mqttLogger.h"
#include "Publisher.h"

using namespace std;
using namespace Politocean;

int main(){
    Controller controller;
    Publisher pub("10.0.0.1", "imuCalibration");

    mqttLogger ptoLogger(&pub);

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
            stringstream ss;
            ss << acc[0] << "\t" << acc[1] << "\t" << acc[2];
            ptoLogger.logInfo(ss.str());
            nReading = 0;
        }else
            nReading++;
        
    }

    return 0;
}