#include <Subscriber.h>

namespace Politocean {

class JoystickSubscriber : public Subscriber {
public:
    static const std::string DFLT_ADDRESS, DFLT_CLIENT_ID, DFLT_TOPIC;

    JoystickSubscriber() : JoystickSubscriber(DFLT_ADDRESS, DFLT_CLIENT_ID, DFLT_TOPIC) {}

    JoystickSubscriber(const std::string& address, const std::string& clientID, const std::string& topic) 
        : Subscriber(address, clientID, topic, &JoystickSubscriber::callback, this) {}

    void callback(const std::string& payload);
};

}