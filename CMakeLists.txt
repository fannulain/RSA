cmake_minimum_required(VERSION 3.10)
project(RSACryptosystem CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

add_executable(rsa_app
    src/BigNum.cpp
    src/RSA.cpp
    src/main.cpp
)
