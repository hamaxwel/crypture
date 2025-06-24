cmake_minimum_required(VERSION 3.10)
project(cpppass)
set(CMAKE_CXX_STANDARD 17)

include_directories(include)

find_package(OpenSSL REQUIRED)

add_executable(cpppass
    src/main.cpp
    src/crypto.cpp
    src/storage.cpp
    src/cli.cpp
)

# nlohmann/json header-only
include(FetchContent)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)
target_link_libraries(cpppass PRIVATE OpenSSL::SSL OpenSSL::Crypto nlohmann_json::nlohmann_json) 