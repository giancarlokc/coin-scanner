
cmake_minimum_required(VERSION 2.8)
 
project(coinScanner)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
 
add_executable(coinScanner coinScanner.cxx)
target_link_libraries(coinScanner ${ITK_LIBRARIES})
