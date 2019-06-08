cmake_minimum_required(VERSION 3.5)
project(Stepper VERSION 1.0.0 LANGUAGES CXX)

add_library(Stepper SHARED
        Stepper.cpp)

add_library(PolitoceanRov::Stepper ALIAS Stepper)

target_link_libraries(Stepper -lpthread PolitoceanRovCommon::Controller)

target_include_directories(Stepper
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(Stepper PRIVATE cxx_auto_type)
target_compile_options(Stepper PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall>)

include(GNUInstallDirs)
install(TARGETS Stepper
        EXPORT PolitoceanRovTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
