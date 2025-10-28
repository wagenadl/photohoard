# CMakeLists.txt                                        -*- Makefile -*-
# Part of PHOTOHOARD, (C) Daniel Wagenaar 2021

# NEW BUILD INSTRUCTIONS (as of version 1.3.x)
# Type:
#   cd build
#   cmake ..
# to build PHOTOHOARD.


######################################################################
cmake_minimum_required(VERSION 3.19)

######################################################################
# Set the project name and version
project(Photohoard VERSION 0.2.0)
set(CMAKE_PROJECT_HOMEPAGE_URL "http://danielwagenaar.net/photohoard")

######################################################################
# Specify the C++ standard [must be done _before_ adding executable!]
set(CMAKE_CXX_STANDARD 17) # rocksdb needs 17
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # I thought this would do the trick...
if (UNIX AND NOT APPLE) 
  add_compile_options(-fPIC) # ... but in fact, this is required on Linux
endif()

# Qt stuff
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# find packages
find_package(Qt5X11Extras QUIET)
if(TARGET Qt5X11Extras)
  find_package(Qt5 REQUIRED COMPONENTS Widgets Sql X11Extras)
else()
  find_package(Qt5 REQUIRED COMPONENTS Widgets Sql)
  # Somehow, find_package(Qt5 ... OPTIONAL_COMPONENTS X11Extras) fails
endif()
find_package(OpenCV REQUIRED)
find_package(exiv2 REQUIRED CONFIG NAMES exiv2)
#find_package(LCMS2 REQUIRED)
find_library(ROCKS_DB_PATH rocksdb REQUIRED)
find_library(LCMS2_PATH lcms2 REQUIRED) 

######################################################################
# Add the executables
add_executable(photohoard WIN32 MACOSX_BUNDLE src/main.cpp)
#target_link_options(photohoard private "$<$<CONFIG:Release>:-s>")

######################################################################
# DEBUG VERSION
#set(CMAKE_BUILD_TYPE DEBUG)

# RELEASE VERSION
#set(CMAKE_BUILD_TYPE RELEASE)

######################################################################
#set_target_properties(photohoard PROPERTIES LINK_FLAGS -s)

include(src/CMakeLists.txt)

# Libraries to link to - Qt must be mentioned
target_link_libraries(photohoard Qt5::Widgets)
target_link_libraries(photohoard Qt5::Sql)
if(TARGET Qt5::X11Extras)
  target_link_libraries(photohoard Qt5::X11Extras)
  target_link_libraries(photohoard X11)
  add_compile_definitions(HAVE_X11EXTRAS)
endif()
target_link_libraries(photohoard "${OpenCV_LIBS}")
target_link_libraries(photohoard exiv2lib)
target_link_libraries(photohoard lcms2)
target_link_libraries(photohoard pthread)
target_link_libraries(photohoard rocksdb)

if (WIN32)
  target_link_libraries(photohoard secur32.lib)
endif()

if (APPLE)
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE INTERNAL "" FORCE)

  #set(MACOSX_BUNDLE_ICON_FILE eln.icns)
  include(GNUInstallDirs)
  install(TARGETS photohoard
      BUNDLE  DESTINATION .
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
  add_custom_command(TARGET photohoard POST_BUILD
     COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/photohoard.app/Contents/Resources
     #COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/src/App/eln.icns
     #   ${CMAKE_CURRENT_BINARY_DIR}/notedeln.app/Contents/Resources
  )
endif()

# Produce configuration files
configure_file(src/config.h.in config.h)

# Include paths - Qt must be mentioned
target_include_directories(photohoard PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(photohoard PUBLIC "${Qt5Widgets_INCLUDE_DIRS}")
target_include_directories(photohoard PUBLIC "${OpenCV_INCLUDE_DIRS}")

######################################################################
# Documentation
if (UNIX AND NOT APPLE)
  add_subdirectory("docs")
endif()

######################################################################
# General packaging / installation
set(CPACK_PACKAGE_VENDOR "Daniel Wagenaar")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Photography Collection Management")
set(CPACK_PACKAGE_DESCRIPTION
  "Photohoard is a program to manage your collection of photographs. Photohoard also contains lots of tools for basic editing.")
set(CPACK_PACKAGE_EXECUTABLES "photohoard;Photohoard")

if(WIN32)
  # NSIS packaging - to be updated
  include(res/WinPack.CMakeLists.txt)
  include(CPack)
elseif(APPLE)
  qt_generate_deploy_app_script(
      TARGET photohoard
      OUTPUT_SCRIPT deploy_script
      NO_UNSUPPORTED_PLATFORM_ERROR
  )
  install(SCRIPT ${deploy_script})
  include(tools/MacDMG.CMakeLists.txt)
elseif(UNIX)
  include(res/UnixInstall.CMakeLists.txt)
  include(res/DebPack.CMakeLists.txt)
  include(CPack)
endif()


