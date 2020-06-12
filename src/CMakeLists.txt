cmake_minimum_required(VERSION 3.2)

include("../cmake/HunterGate.cmake")
HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.254.tar.gz"
        SHA1 "017e688b9848c1f695303e8cfe9f35d40e154112"
)

project(main VERSION 1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

hunter_add_package(Boost COMPONENTS date_time)
find_package(Boost CONFIG REQUIRED date_time)

set(Boost_USE_MULTITHREADED TRUE)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options (-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options (-fcolor-diagnostics)
endif ()

add_executable(main main.cpp framework/utils.cpp)

target_link_libraries(main PUBLIC
        Boost::date_time
        OpenSSL::SSL OpenSSL::Crypto
    )
target_compile_options(main PRIVATE -Wall) #  -Wextra

