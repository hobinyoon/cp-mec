CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(EXE_NAME hide)

PROJECT(${EXE_NAME})

FILE(GLOB_RECURSE sources src/*.cpp)

ADD_EXECUTABLE(${EXE_NAME} ${sources})

TARGET_COMPILE_OPTIONS(${EXE_NAME} PUBLIC -Wall -Werror -std=gnu++11)

# http://stackoverflow.com/questions/10984442/how-to-detect-c11-support-of-a-compiler-with-cmake/20165220#20165220
SET_PROPERTY(TARGET ${EXE_NAME} PROPERTY CXX_STANDARD 11)

FIND_PACKAGE(Boost COMPONENTS
	date_time
	filesystem
	program_options
	regex
	system
	timer
	REQUIRED)

TARGET_LINK_LIBRARIES(${EXE_NAME} PUBLIC
	${Boost_LIBRARIES}
	yaml-cpp
	)
