CMAKE_MINIMUM_REQUIRED(VERSION 3.7.0)
FILE(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
ADD_EXECUTABLE(main ${SRC_FILES})
SET_TARGET_PROPERTIES(main PROPERTIES CXX_STANDARD 17)
TARGET_LINK_LIBRARIES(main game)

IF(APPLE)
    FIND_LIBRARY(CPPEXPERIMENTAL "libc++experimental.a")
    IF(CPPEXPERIMENTAL)
        TARGET_LINK_LIBRARIES(main ${CPPEXPERIMENTAL})
    ENDIF()
ENDIF()
