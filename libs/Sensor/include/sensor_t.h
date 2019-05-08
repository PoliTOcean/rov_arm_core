/**
 * @author: pettinz
 */

#ifndef SENSOR_T_H
#define SENSOR_T_H

namespace Politocean {

/**
 * This enum class acts as a sensors list. Insert here and remove sensors, maintaining @First and @Last variables
 * updated, for a correct operation of the iterator.
 *
 * The class is iterable, see functions below.
 */
enum class sensor_t {
    ROLL,
    PITCH,
    TEMPERATURE,
    PRESSION,

    First=ROLL,
    Last=PRESSION
};

inline sensor_t operator++ (sensor_t &s) { return s = (sensor_t)(((int)(s) +1)); }
inline sensor_t operator* (sensor_t &s) { return s; }
inline sensor_t begin(sensor_t r) { return sensor_t::First; }
inline sensor_t end(sensor_t r) { sensor_t l = sensor_t::Last; return ++l; }

}

#endif //SENSOR_T_H