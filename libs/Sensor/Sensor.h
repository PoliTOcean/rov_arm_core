/**
 * @author: pettinz
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <iostream>

#include "include/sensor_t.h"

namespace Politocean {

template <class T>
class Sensor {
    sensor_t type_;
    T value_;

public:
    Sensor(sensor_t type, T value) : type_(type), value_(value) {}

    void setValue(T value) { value_ = value; }
    T getValue() { return value_ }
    sensor_t getType();

    /*
     * Overloading the operator << to properly print a sensor object
     */
    friend std::ostream& operator<< (std::ostream& os, const Sensor& s) {
        switch (s.type_) {
            case sensor_t::ROLL:
                os << "ROLL";
                break;
            case sensor_t::PITCH:
                os << "PITCH";
                break;
            case sensor_t::TEMPERATURE:
                os << "TEMPERATURE";
                break;
            case sensor_t::PRESSION:
                os << "PRESSION";
                break;
        }
        os << ": " << static_cast<unsigned>(s.value_);

        return os;
    }

};

}

#endif //SENSOR_H
