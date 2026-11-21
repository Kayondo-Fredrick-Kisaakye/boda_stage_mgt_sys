cmake_minimum_required(VERSION 3.16)
project(BodaStageManager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.18.7
)
FetchContent_MakeAvailable(httplib)

find_package(libpqxx REQUIRED)
find_package(Threads REQUIRED)

add_executable(boda_stage
  src/main.cpp
  src/models.cpp
  src/database.cpp
  src/web_server.cpp
)

target_include_directories(boda_stage PRIVATE src)
target_link_libraries(boda_stage PRIVATE httplib::httplib libpqxx::pqxx Threads::Threads)
