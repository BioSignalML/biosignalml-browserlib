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
elseif(WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# Tell CMake to run uic when necessary:
set(CMAKE_AUTOUIC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Qt5 packages find their own dependencies.
find_package(Qt5Core    REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui     REQUIRED)

# Keep track of some information about Qt
set(QT_BINARY_DIR ${_qt5Widgets_install_prefix}/bin)
set(QT_LIBRARY_DIR ${_qt5Widgets_install_prefix}/lib)
set(QT_PLUGINS_DIR ${_qt5Widgets_install_prefix}/plugins)
set(QT_VERSION ${Qt5Widgets_VERSION})
set(QT_VERSION_MAJOR ${Qt5Widgets_VERSION_MAJOR})
set(QT_VERSION_MINOR ${Qt5Widgets_VERSION_MINOR})
set(QT_VERSION_PATCH ${Qt5Widgets_VERSION_PATCH})


add_subdirectory(src)

include_directories(${INCLUDES})
add_library(browserlib SHARED ${SOURCES})


message("Libs: ${LIBRARIES}")
target_link_libraries(browserlib ${LIBRARIES})

set(QT_LIBRARIES Core Widgets Gui)
qt5_use_modules(browserlib ${QT_LIBRARIES})

if(APPLE)
  foreach(QT_LIBRARY ${QT_LIBRARIES})
    set(QT_LIBRARY_FILENAME Qt${QT_LIBRARY}.framework/Versions/${QT_VERSION_MAJOR}/Qt${QT_LIBRARY})
    add_custom_command(TARGET browserlib POST_BUILD
               COMMAND install_name_tool -change ${QT_LIBRARY_DIR}/${QT_LIBRARY_FILENAME}
                               @rpath/${QT_LIBRARY_FILENAME}
                               ${CMAKE_BINARY_DIR}/libbrowserlib.dylib)
  endforeach()
endif()


add_executable(browser src/main.cpp)
target_link_libraries(browser browserlib)

