cmake_minimum_required(VERSION 3.5)
project(JoystickSubscriber VERSION 1.0.0 LANGUAGES CXX)

add_library(JoystickSubscriber
        JoystickSubscriber.cpp)

add_library(PolitoceanRovArmCore::JoystickSubscriber ALIAS JoystickSubscriber)

target_include_directories(JoystickSubscriber
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(JoystickSubscriber PolitoceanCommon::Subscriber)

target_compile_features(JoystickSubscriber PRIVATE cxx_auto_type)
target_compile_options(JoystickSubscriber PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall>)

include(GNUInstallDirs)
install(TARGETS JoystickSubscriber
        EXPORT PolitoceanRovArmCoreTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
