cmake_minimum_required(VERSION 3.1)

project(browser)

FIND_PACKAGE(biosignalml REQUIRED)
set(INCLUDES ${INCLUDES} ${biosignalml_INCLUDE_DIR})
set(LIBRARIES ${LIBRARIES} ${biosignalml_LIBRARY})

FIND_PACKAGE(typedobject REQUIRED)
set(INCLUDES ${INCLUDES} ${typedobject_INCLUDE_DIR})
set(LIBRARIES ${LIBRARIES} ${typedobject_LIBRARY})

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

add_subdirectory(src)

include_directories(${INCLUDES})
add_executable(browser ${SOURCES})

qt5_use_modules(browser Widgets Gui)
target_link_libraries(browser ${LIBRARIES})
