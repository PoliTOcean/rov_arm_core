#include "JoystickSubscriber.h"
#include "json.hpp"

namespace Politocean {

const std::string JoystickSubscriber::DFLT_ADDRESS   { "tcp://localhost:1883" };
const std::string JoystickSubscriber::DFLT_CLIENT_ID { "JoystickSubscriber" };
const std::string JoystickSubscriber::DFLT_TOPIC     { "JoystickTopic" };

void JoystickSubscriber::callback(const std::string& payload)
{
    // TODO: Elaborate payload, filter commands and send via SPI to microcontroller

    auto c_map = nlohmann::json::parse(payload);
    
    axes_       = c_map["axes"].get<std::vector<int>>();
    buttons_    = c_map["axes"].get<std::vector<int>>();

    std::cout << axes_[0] << std::endl;
}

}