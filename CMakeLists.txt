cmake_minimum_required(VERSION 3.10.0)
project(tubekit)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -Wall -O0")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

message(${PROJECT_SOURCE_DIR}/src)

#add include for src/
include_directories(${PROJECT_SOURCE_DIR}/src)

#add include for protocol/
include_directories(${PROJECT_SOURCE_DIR}/protocol)

#add include for external/
include_directories(${PROJECT_SOURCE_DIR}/external)

#for cpp file
file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

#for protocol cc file
file(GLOB_RECURSE SOURCE_PROTOCOL_FILES "${PROJECT_SOURCE_DIR}/protocol/*.pb.cc")
list(APPEND SOURCE_FILES ${SOURCE_PROTOCOL_FILES})

# for plugin
#file(GLOB_RECURSE SOURCE_PLUGIN "${PROJECT_SOURCE_DIR}/src/plugin/*.cpp")
#list(REMOVE_ITEM SOURCE_FILES ${SOURCE_PLUGIN})
#foreach(item ${SOURCE_FILES})
#    message("SourceFile: ${item}")
#endforeach()

add_executable(${PROJECT_NAME} src/main.cpp ${SOURCE_FILES})

#for src/plugin CMakeLists.txt
#add_subdirectory(${PROJECT_SOURCE_DIR}/src/plugin)

#for external/ CMakeLists.txt
add_subdirectory(${PROJECT_SOURCE_DIR}/external)

# for link external gen .so lib
set(EXTERNAL_LIB tubekit-http-parser tubekit-inifile tubekit-log tubekit-timer tubekit-xml tubekit-buffer tubekit-lua tubekit-zlib tubekit-json)

target_link_libraries(${PROJECT_NAME} pthread dl stdc++fs protobuf ssl crypto ${EXTERNAL_LIB})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)
