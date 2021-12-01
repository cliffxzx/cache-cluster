cmake_minimum_required(VERSION 3.0.0)
project(cache-cluster VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
# include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_TOOLCHAIN_FILE /Users/cliff/Code/vcpkg/scripts/buildsystems/vcpkg.cmake)
include(/Users/cliff/Code/vcpkg/scripts/buildsystems/vcpkg.cmake)
find_package(Boost REQUIRED COMPONENTS system serialization filesystem date_time)

include(CTest)
enable_testing()

set(SOURCE_FILES
  "gossip_protocol.hpp"
  "gossip_protocol.cpp"
  "utils.hpp"
  "utils.cpp"
  "message.hpp"
  "message.cpp"
  "main.cpp"
)

add_executable(cache-cluster ${SOURCE_FILES})
target_link_libraries(cache-cluster PRIVATE Boost::boost
                                            ${Boost_SYSTEM_LIBRARY}
                                            ${Boost_SERIALIZATION_LIBRARY})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
