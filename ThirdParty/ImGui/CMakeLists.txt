file(GLOB_RECURSE SRC *.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Point to a couple of dirs because ImGUI's config points to AnKi headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../..)
if(NOT MSVC)
	add_definitions("-std=c++14")
endif()

add_library(AnKiImGui ${SRC})

