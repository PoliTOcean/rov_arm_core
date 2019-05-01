/**
 * @author pettinz
 */

#include "JoystickSubscriber.h"
#include "json.hpp"

#include "Commands.h"

namespace Politocean
{

const std::string JoystickSubscriber::DFLT_ADDRESS   { "tcp://localhost:1883" };
const std::string JoystickSubscriber::DFLT_CLIENT_ID { "JoystickSubscriber" };
const std::string JoystickSubscriber::DFLT_TOPIC     { "JoystickTopic" };

void JoystickSubscriber::callback(const std::string& payload)
{
    auto c_map = nlohmann::json::parse(payload);
    
    std::vector<int> axes       = c_map["axes"];
    std::vector<int> buttons    = c_map["axes"];

    axesBuffer_     = {
        axes[COMMANDS::AXIS::X],
        axes[COMMANDS::AXIS::Y],
        axes[COMMANDS::AXIS::RZ]
    };

    buttonsBuffer_  = {
        buttons[COMMANDS::BUTTONS::MOTORS_ON],
        buttons[COMMANDS::BUTTONS::MOTORS_OFF],
        buttons[COMMANDS::BUTTONS::VDOWN],
        buttons[COMMANDS::BUTTONS::WRIST],
        buttons[COMMANDS::BUTTONS::RESET],
        buttons[COMMANDS::BUTTONS::VUP],
        buttons[COMMANDS::BUTTONS::MEDIUM_FAST],
        buttons[COMMANDS::BUTTONS::SLOW]
    };

    // Using shifts to convert a binary value vector to a decimal integer
    int button = 0;
    for (auto it = buttonsBuffer_.rbegin(); it != buttonsBuffer_.rend(); it++)
        button += *it << (it - buttonsBuffer_.rbegin());

    unsigned char data = (current_ >= axesBuffer_.size()) ? static_cast<unsigned char>(button) : axesBuffer_[current_];
    sensors_[current_].setValue(controller_.SPIDataRW(data));

    if (++current_ > sensors_.size())
        current_ = 0;
}

}