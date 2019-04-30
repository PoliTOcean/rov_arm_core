#include <iostream>

#include "JoystickSubscriber.h"
#include "PolitoceanConstants.h"
#include "logger.h"
#include "mqttLogger.h"
#include "Publisher.h"

using namespace Politocean;
using namespace std;
using namespace Politocean::Constants;

Publisher pub("127.0.0.1", Rov::CLIENT_ID);
mqttLogger ptoLogger(&pub);

int main(int argc, const char *argv[])
{
    logger::enableLevel(logger::DEBUG, true);
    try {
        pub.connect();
    
        JoystickSubscriber subscriber;

        subscriber.wait();
    } catch(const mqtt::exception& e) {
        ptoLogger.logError(e);
    } catch(const std::exception& e) {
        ptoLogger.logError(e);
    }
    

    return 0;
}