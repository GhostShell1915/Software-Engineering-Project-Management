cmake_minimum_required(VERSION 3.14)
project(PasswordManager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# OpenSSL
find_package(OpenSSL REQUIRED)

add_executable(password_manager
    src/main.cpp
    src/crypto.cpp
    src/vault.cpp
)

target_include_directories(password_manager PRIVATE include)
target_link_libraries(password_manager PRIVATE OpenSSL::Crypto)

# Warnings
if(MSVC)
    target_compile_options(password_manager PRIVATE /W4)
else()
    target_compile_options(password_manager PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install (optional)
install(TARGETS password_manager RUNTIME DESTINATION bin)
