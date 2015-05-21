
cmake_minimum_required(VERSION 2.8)
 
project(trabalho)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
 
add_executable(trabalho trabalho.cxx)
target_link_libraries(trabalho ${ITK_LIBRARIES})
