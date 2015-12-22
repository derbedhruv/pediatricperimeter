project(pediperi)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -std=c++0x -O2")

add_executable(${PROJECT_NAME} main.cpp)

find_package(SDL2 REQUIRED)
find_package(OpenCV REQUIRED)

set(SDL2_INCLUDE_DIR /Library/Frameworks/SDL2.framework/Headers) #Total hack

INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIR} /opt/local/include/)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL2_LIBRARY} ${OpenCV_LIBS})