/**
 * @author pettinz
 */

#include "JoystickSubscriber.h"
#include "json.hpp"

#include "Commands.h"
#include "PolitoceanUtils.hpp"

namespace Politocean
{

const std::string JoystickSubscriber::DFLT_ADDRESS   { "tcp://localhost:1883" };
const std::string JoystickSubscriber::DFLT_CLIENT_ID { "JoystickSubscriber" };
const std::string JoystickSubscriber::DFLT_TOPIC     { "JoystickTopic" };

void JoystickSubscriber::callback(const std::string& payload)
{
    auto c_map = nlohmann::json::parse(payload);
    
    axes_   = c_map["axes"].get<std::vector<int>>();
    button_ = c_map["button"];
    
    std::vector<int> axesBuffer = {
        axes_[COMMANDS::AXIS::X],
        axes_[COMMANDS::AXIS::Y],
        axes_[COMMANDS::AXIS::RZ]
    };

    unsigned char data = (current_ >= axesBuffer.size()) ? button_: map(axesBuffer[current_], 0, INT_MAX);
    sensors_[current_].setValue(controller_.SPIDataRW(data));

    if (++current_ > sensors_.size())
        current_ = 0;

}

}
