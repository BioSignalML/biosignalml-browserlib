cmake_minimum_required(VERSION 3.1)

project(browser)


if(UNIX)
  add_definitions(-std=c++11)  # Use C++11
endif()

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# Tell CMake to run uic when necessary:
set(CMAKE_AUTOUIC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Qt5 packages find their own dependencies.
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui     REQUIRED)



set(SOURCES
  nrange.cpp
  annotationdialog.cpp
  chartplot.cpp
  browser.cpp 
  )

add_executable(browser ${SOURCES})
qt5_use_modules(browser Widgets Gui)

