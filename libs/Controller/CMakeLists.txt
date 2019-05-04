cmake_minimum_required(VERSION 3.5)
project(Controller VERSION 1.0.0 LANGUAGES CXX)

add_library(Controller
        Controller.cpp)

add_library(PolitoceanRovArmCore::Controller ALIAS Controller)

target_link_libraries(Controller -lwiringPi -lcrypt -lrt)

target_include_directories(Controller
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(Controller PRIVATE cxx_auto_type)
target_compile_options(Controller PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall>)

include(GNUInstallDirs)
install(TARGETS Controller
        EXPORT PolitoceanRovArmCoreTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
