cmake_minimum_required(VERSION 3.0.0)
project(cache-cluster VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_TOOLCHAIN_FILE /Users/cliff/Code/vcpkg/scripts/buildsystems/vcpkg.cmake)
include(/Users/cliff/Code/vcpkg/scripts/buildsystems/vcpkg.cmake)
find_package(Boost REQUIRED COMPONENTS system serialization filesystem date_time log program_options)

include(CTest)
enable_testing()

set(SOURCE_FILES
"member.hpp"
"member.cpp"
"message.hpp"
"message.cpp"
"gossip.hpp"
"gossip.cpp"
"main.cpp"
# "test.cpp"
)


add_executable(cache-cluster ${SOURCE_FILES})
target_link_libraries(cache-cluster PRIVATE Boost::boost ${Boost_LIBRARIES})
target_include_directories(cache-cluster PRIVATE ${Boost_INCLUDE_DIRS})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
target_compile_options(cache-cluster PRIVATE -Wno-potentially-evaluated-expression)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
# using GCC
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
# using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
# using Visual Studio C++
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
