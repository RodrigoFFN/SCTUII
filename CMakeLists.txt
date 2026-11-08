cmake_minimum_required(VERSION 3.16)
project(SimTrrafic VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(simtrrafic src/main.cpp)
target_include_directories(simtrrafic PRIVATE include)
target_link_libraries(simtrrafic PRIVATE Threads::Threads)

include(CTest)
if(BUILD_TESTING)
    add_executable(test_concurrencia tests/test_concurrencia.cpp)
    target_include_directories(test_concurrencia PRIVATE include)
    target_link_libraries(test_concurrencia PRIVATE Threads::Threads)
    add_test(NAME concurrencia_basica COMMAND test_concurrencia)
endif()

