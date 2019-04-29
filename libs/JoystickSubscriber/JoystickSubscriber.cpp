#include "JoystickSubscriber.h"

namespace Politocean {

using namespace std;

const string JoystickSubscriber::DFLT_ADDRESS   { "tcp://localhost:1883" };
const string JoystickSubscriber::DFLT_CLIENT_ID { "JoystickSubscriber" };
const string JoystickSubscriber::DFLT_TOPIC     { "JoystickTopic" };

void JoystickSubscriber::callback(const string& payload)
{
    cout << payload << endl;
}

}