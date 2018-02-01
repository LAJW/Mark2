CMAKE_MINIMUM_REQUIRED(VERSION 3.7.0)
FILE(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
ADD_LIBRARY(game STATIC ${SRC_FILES})
TARGET_COMPILE_OPTIONS(game
	PUBLIC /std:c++17)
TARGET_COMPILE_DEFINITIONS(game
	PUBLIC
	SFML_STATIC # Force SFML to compile for static libraries
	_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS # YAML Uses deprecated std::iterator
)
TARGET_INCLUDE_DIRECTORIES(game PUBLIC
	../SFML/include
	../yaml-cpp/include
	${SFML_INCLUDE_DIRECTORY}
	${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(game INTERFACE
	yaml-cpp
	opengl32
	winmm
	debug sfml-graphics-s-d
	optimized sfml-graphics-s
	debug sfml-window-s-d
	optimized sfml-window-s
	debug sfml-system-s-d
	optimized sfml-system-s
	debug sfml-main-d
	optimized sfml-main
)
