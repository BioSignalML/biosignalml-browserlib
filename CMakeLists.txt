cmake_minimum_required(VERSION 3.1)

project(browser)


if(UNIX)
  add_definitions(-std=c++11)  # Use C++11
endif()

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Qt5 packages find their own dependencies.
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui     REQUIRED)



add_executable(browser nrange.cpp browser.cpp chartplot.cpp)

qt5_use_modules(browser Widgets Gui)

