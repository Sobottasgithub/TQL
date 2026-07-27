cmake_minimum_required(VERSION 3.10)
project(tql-test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "./*.cpp" "./*.h")

add_executable(tql-test ${SOURCES})

target_link_libraries(tql-test PRIVATE tql)
target_link_libraries(tql-test PRIVATE tablog)

install(TARGETS tql-test DESTINATION bin)
