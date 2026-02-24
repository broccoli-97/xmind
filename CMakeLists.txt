cmake_minimum_required(VERSION 3.16)
project(xmind VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_STATIC "Build a fully static executable" OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MindMapScene.cpp
    src/MindMapView.cpp
    src/NodeItem.cpp
    src/EdgeItem.cpp
)

set(HEADERS
    src/MainWindow.h
    src/MindMapScene.h
    src/MindMapView.h
    src/NodeItem.h
    src/EdgeItem.h
)

if(WIN32)
    # WIN32 flag prevents console window from appearing
    add_executable(xmind WIN32 ${SOURCES} ${HEADERS})
else()
    add_executable(xmind ${SOURCES} ${HEADERS})
endif()

target_link_libraries(xmind PRIVATE Qt6::Widgets)
target_include_directories(xmind PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Static linking for Windows
if(BUILD_STATIC AND WIN32)
    set_target_properties(xmind PROPERTIES
        LINK_FLAGS "-static -static-libgcc -static-libstdc++"
    )
endif()

install(TARGETS xmind DESTINATION bin)
