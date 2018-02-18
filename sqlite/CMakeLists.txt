cmake_minimum_required (VERSION 2.8)

project (sqlite_efficiency)

#add_compile_options(-lpthread -ldl)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -ldl")

aux_source_directory(. DIR_SRCS) 

add_executable(sqlite_efficiency ${DIR_SRCS})

TARGET_LINK_LIBRARIES(sqlite_efficiency pthread dl)