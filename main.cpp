#include <iostream>

#include "JoystickSubscriber.h"

using namespace Politocean;
using namespace std;

int main(int argc, const char *argv[])
{
    try {
        JoystickSubscriber subscriber(JoystickSubscriber::DFLT_ADDRESS, JoystickSubscriber::DFLT_CLIENT_ID, JoystickSubscriber::DFLT_TOPIC);
        
        subscriber.connect();
        subscriber.listen(&JoystickSubscriber::callback, &subscriber)->join();
        subscriber.disconnect();
    } catch(const mqtt::exception& e) {
        cerr << e.what() << endl;
    } catch(const std::exception& e) {
        cerr << e.what() << endl;
    }
    

    return 0;
}