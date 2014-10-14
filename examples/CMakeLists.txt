CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(classify)
#FIND_PACKAGE(OpenCV REQUIRED)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../include)
SET(CPP_FILES classify/classify.cpp)

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") # GCC on Linux
	# We need C++11 support
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -O3") # Enable lots of warning flags and optimization
	IF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x") # C++11 support on gcc v 4.7--
	ELSE()
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") # C++11 support on gcc v 4.7+
	ENDIF()
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel") # Intel C++ compiler
	# We need C++11 support
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wno-unused-parameter -O2") # Enable lots of warning flags and optimization
	# Other optimizations. See https://software.intel.com/en-us/articles/step-by-step-optimizing-with-intel-c-compiler
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -xAVX -ipo")
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -xAVX -ipo -prof-gen -prof-dir=.")
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -xAVX -ipo -prof-use -prof-dir=.")
ENDIF()

	
ADD_EXECUTABLE(${PROJECT_NAME} ${CPP_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME})
