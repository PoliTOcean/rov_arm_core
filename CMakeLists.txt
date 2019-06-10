cmake_minimum_required(VERSION 3.5)
project(PolitoceanRov)

set(CMAKE_CXX_STANDARD 11)

if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "
	In-source builds are not allowed.
	Please, create a separate directory for build files (e.g. build/).
	Delete CMakeCache.txt, make a separate dir and try again.
	")
endif()

# Add politocean common libs
add_subdirectory(politocean_common)
include_directories(politocean_common/include)

add_subdirectory(rov_common)
include_directories(rov_common/include)

add_subdirectory(libs)

include_directories(include)

# Add executable
add_executable(PolitoceanATMega src/ATMegaController.cpp)
add_executable(PolitoceanSkeleton src/Skeleton.cpp)

# Linking the libraries
target_link_libraries(PolitoceanATMega -lpthread
    PolitoceanRovCommon::Controller

    PolitoceanCommon::Sensor
    PolitoceanCommon::mqttLogger
    PolitoceanCommon::MqttClient
    PolitoceanCommon::Component
)


target_link_libraries(PolitoceanSkeleton -lpthread
    PolitoceanRovCommon::Controller
    PolitoceanRov::Stepper
    PolitoceanRov::DCMotor
    
    PolitoceanCommon::MqttClient
    PolitoceanCommon::Component
    PolitoceanCommon::mqttLogger
)

# install
set(CMAKE_INSTALL_PREFIX:PATH /usr)
include(GNUInstallDirs)

install ( TARGETS ${PolitoceanRov_LIBS} ${PolitoceanCommon_LIBS} ${PolitoceanRovCommon_LIBS}
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install ( TARGETS PolitoceanSkeleton PolitoceanATMega
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
          PERMISSIONS WORLD_READ WORLD_EXECUTE )