cmake_minimum_required(VERSION 3.10)
project(YadroTestQam CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

add_library(signal_processing STATIC
    src/signal_processing/qam_modulator.cpp
    src/signal_processing/qam_demodulator.cpp
    src/channel/awgn_channel.cpp
)

target_include_directories(signal_processing PUBLIC include)

add_executable(simulation src/main.cpp)

target_link_libraries(simulation PRIVATE signal_processing)