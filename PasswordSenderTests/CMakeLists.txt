cmake_minimum_required(VERSION 3.5)

project(PasswordSenderTestCase LANGUAGES CXX)


find_package(Qt5Test REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(..)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
enable_testing()

add_executable(PasswordSenderTestCase tst_passwordsendertestcase.cpp
    ../qfileencrypted.cpp
    mynamedtempfile.cpp)
add_test(PasswordSenderTestCase COMMAND PasswordSenderTestCase)

target_link_libraries(PasswordSenderTestCase PRIVATE Qt5::Test crypto)

