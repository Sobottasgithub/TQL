cmake_minimum_required(VERSION 3.10)
project(tql)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")
# file(GLOB_RECURSE HEADERS "include/*.h")

add_library(tql SHARED ${SOURCES})

target_include_directories(tql PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(tql PRIVATE tablog)
find_package(Arrow REQUIRED)
target_link_libraries(tql PRIVATE Arrow::arrow_shared)

install(TARGETS tql DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
