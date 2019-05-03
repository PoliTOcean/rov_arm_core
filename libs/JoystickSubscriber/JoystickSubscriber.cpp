/**
 * @author pettinz
 */

#include "JoystickSubscriber.h"
#include "json.hpp"

#include "Commands.h"
#include "PolitoceanConstants.h"
#include "PolitoceanUtils.hpp"

namespace Politocean
{

const std::string JoystickSubscriber::DFLT_ADDRESS   { "tcp://localhost:1883" };
const std::string JoystickSubscriber::DFLT_CLIENT_ID { "JoystickSubscriber" };

void JoystickSubscriber::listenForButtons(const std::string& payload)
{
    button_ = static_cast<unsigned char>(std::stoi(payload));
}

void JoystickSubscriber::listenForAxes(const std::string& payload)
{
    auto c_map = nlohmann::json::parse(payload);

    axes_ = c_map["axes"].get<std::vector<int>>();
}

void JoystickSubscriber::startListening()
{
    subscribeTo(Constants::Topics::JOYSTICK_AXES, &JoystickSubscriber::listenForAxes, this);
    subscribeTo(Constants::Topics::JOYSTICK_BUTTONS, &JoystickSubscriber::listenForButtons, this);
    
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

void JoystickSubscriber::stopListening()
{
    unsubscribeFrom({ Constants::Topics::JOYSTICK_BUTTONS, Constants::Topics::JOYSTICK_AXES });
}

}
