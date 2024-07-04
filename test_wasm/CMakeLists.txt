cmake_minimum_required(VERSION 3.8)

project(test_wasm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXECUTABLE_SUFFIX ".wasm")

# Calc
# could use a static lib (see README)
add_executable(calc calc.cpp calc.hpp)
set_target_properties(calc PROPERTIES COMPILE_FLAGS "-Os -s SIDE_MODULE=1 ")
set_target_properties(calc PROPERTIES LINK_FLAGS    "-Os -s WASM=1 -s SIDE_MODULE=1 -s STANDALONE_WASM --no-entry")

# Qml
# set(qt_libs
#   Qt5::Qml
#   Qt5::Quick
#   Qt5::Gui
#   Qt5::Core
#   Qt5::Widgets
#   Qt5::QuickControls2
# )
# find_package(Qt5 CONFIG REQUIRED COMPONENTS Qml Quick Gui Core Widgets QuickControls2)
# add_executable(qml qml.cpp)
# set_target_properties(qml PROPERTIES
#   RESOURCE qml.qrc
# )
# set_target_properties(qml PROPERTIES
#     AUTOMOC ON
#     AUTOUIC ON
#     AUTORCC ON
# )
# target_link_libraries(qml PUBLIC ${qt_libs})
