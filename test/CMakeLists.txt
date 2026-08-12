cmake_minimum_required(VERSION 3.10)
project(tql-test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "./*.cpp" "./*.h")

find_package(PkgConfig REQUIRED)
pkg_check_modules(READLINE REQUIRED readline)

add_executable(tql-test ${SOURCES})



target_include_directories(tql-test PRIVATE ${READLINE_INCLUDE_DIRS})
target_link_directories(tql-test PRIVATE ${READLINE_LIBRARY_DIRS})
# Link readline libraries
target_link_libraries(tql-test PRIVATE ${READLINE_LIBRARIES})


target_link_libraries(tql-test PRIVATE tql)
target_link_libraries(tql-test PRIVATE tablog)
find_package(Arrow REQUIRED)
target_link_libraries(tql-test PRIVATE Arrow::arrow_shared)

install(TARGETS tql-test DESTINATION bin)
