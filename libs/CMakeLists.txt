cmake_minimum_required(VERSION 3.5)
project(PolitoceanRovArmCore)

# Add here all libraries' directories
#add_subdirectory(name_of_directory)
add_subdirectory(JoystickSubscriber)


# Install and export (do not touch this part)
install(
  EXPORT PolitoceanRovArmCoreTargets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PolitoceanRovArmCore
  NAMESPACE PolitoceanRovArmCore::
  FILE PolitoceanRovArmCoreTargets.cmake
  )

include(CMakePackageConfigHelpers)
configure_package_config_file( 
  "Config.cmake.in" 
  "PolitoceanRovArmCoreConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PolitoceanRovArmCore
  PATH_VARS
    CMAKE_INSTALL_LIBDIR
  )

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/PolitoceanRovArmCoreConfigVersion.cmake
  VERSION 1.0.0
  COMPATIBILITY SameMajorVersion
  )

### Install Config and ConfigVersion files
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/PolitoceanRovArmCoreConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/PolitoceanRovArmCoreConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/PolitoceanRovArmCore"
  )
