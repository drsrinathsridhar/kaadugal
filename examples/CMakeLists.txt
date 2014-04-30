CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(classify)
#FIND_PACKAGE(OpenCV REQUIRED)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../include)
SET(CPP_FILES classify/classify.cpp)

# We need C++11 support
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -O3") # Disable all warnings and enable optimization
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -O3") # Enable lots of warning flags and optimization
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")# C++11 support on gcc

	
ADD_EXECUTABLE(${PROJECT_NAME} ${CPP_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME})
