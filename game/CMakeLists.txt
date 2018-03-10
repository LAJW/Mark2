CMAKE_MINIMUM_REQUIRED(VERSION 3.7.0)
FILE(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/**.cpp)
ADD_LIBRARY(game STATIC ${SRC_FILES})
SET_TARGET_PROPERTIES(game PROPERTIES CXX_STANDARD 17)
TARGET_COMPILE_DEFINITIONS(game
	PUBLIC
	SFML_STATIC # Force SFML to compile for static libraries
	_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS # YAML Uses deprecated std::iterator
)
TARGET_INCLUDE_DIRECTORIES(game SYSTEM PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../SFML/include
	${CMAKE_CURRENT_SOURCE_DIR}/../yaml-cpp/include)
TARGET_INCLUDE_DIRECTORIES(game PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR})

find_package(OPENGL REQUIRED)

TARGET_LINK_LIBRARIES(game INTERFACE
	yaml-cpp
	${OPENGL_LIBRARIES}
)

if(APPLE)
	TARGET_LINK_LIBRARIES(game INTERFACE
		sfml-graphics
		sfml-window
		sfml-system
	)
ENDIF()

IF(WIN32)
	TARGET_LINK_LIBRARIES(game INTERFACE
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
ENDIF()
