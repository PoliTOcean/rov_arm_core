cmake_minimum_required(VERSION 3.5)
project(PolitoceanRovArmCore)

set(CMAKE_CXX_STANDARD 11)

# Add politocean common libs
add_subdirectory(politocean_common)
include_directories(politocean_common/include)

# Add project internal libs
add_subdirectory(libs)

include_directories(include)

# Add executable
add_executable(JoystickSub src/joystickSub.cpp)

# Linking the libraries
target_link_libraries(JoystickSub 
    # Add here all the libraries as:
    # Name_of_the_library       --- if it's internal (e.g. Subscriber)
    # -lname_of_the_library     --- if it's a standard c++ library (e.g. -lpthread)
    PolitoceanRovArmCore::JoystickSubscriber
    PolitoceanCommon::mqttLogger
    PolitoceanCommon::logger
    PolitoceanCommon::Publisher)