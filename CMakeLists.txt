cmake_minimum_required(VERSION 3.10)
project(tql)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DEF_TQL "configure library" ON)
option(DEF_TEST "configure test" ON)

if(DEF_TQL)
    add_subdirectory(lib/tql)
endif()

if(DEF_TEST)
    add_subdirectory(test)
endif()
