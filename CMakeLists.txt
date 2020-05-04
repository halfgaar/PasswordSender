cmake_minimum_required(VERSION 3.5)

project(PasswordSender LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED Core Network)

set(ENABLE_DEBUG ON)

set(DISABLE_EXAMPLE ON)
add_subdirectory(qfcgi)

add_executable(PasswordSender main.cpp
    qfcgiapp.cpp
    requestdownloader.cpp
    parsedrequest.cpp
  )

target_link_libraries(PasswordSender qfcgi Qt5::Core Qt5::Network vmime)

install(TARGETS PasswordSender
  RUNTIME DESTINATION /home/halfgaar/
)
