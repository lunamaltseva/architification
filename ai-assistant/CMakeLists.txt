cmake_minimum_required(VERSION 3.16)
project(ArchAssistant VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)

qt_add_executable(ai-assistant
    main.cpp
    assistant.cpp
)

target_link_libraries(ai-assistant PRIVATE Qt6::Widgets Qt6::Core)
