cmake_minimum_required(VERSION 3.16)
project(RestartPopup VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)

qt_add_executable(restart-popup
    main.cpp
    restartpopup.cpp
)

target_link_libraries(restart-popup PRIVATE Qt6::Widgets Qt6::Core)
