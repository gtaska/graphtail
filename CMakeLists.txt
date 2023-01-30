cmake_minimum_required (VERSION 3.19)

project(graphtail)

macro(graphtail_option OPTION_NAME OPTION_TEXT OPTION_DEFAULT)
    option(${OPTION_NAME} ${OPTION_TEXT} ${OPTION_DEFAULT})
    if(DEFINED ENV{${OPTION_NAME}})
        # Allow overriding the option through an environment variable
        set(${OPTION_NAME} $ENV{${OPTION_NAME}})
    endif()
    if(${OPTION_NAME})
        add_definitions(-D${OPTION_NAME})
    endif()
    message(STATUS "${OPTION_NAME}: ${${OPTION_NAME}}")
endmacro()

graphtail_option(GRAPHTAIL_FORCE_USE_SYSTEM_SDL "Force use system SDL." OFF)

set(CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake"
	${CMAKE_MODULE_PATH})
	
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)

FetchContent_Declare(
	brotli
	GIT_REPOSITORY https://github.com/google/brotli
	GIT_TAG v1.0.9)
FetchContent_MakeAvailable(brotli)	

if(GRAPHTAIL_FORCE_USE_SYSTEM_SDL)
	find_package(SDL2)
	if(SDL_FOUND)
		message(STATUS "SDL2 ${SDL2_VERSION_STRING} found on this system.")
	else()
		message(FATAL_ERROR "SDL2 not found on this system.")
	endif()
else()
	FetchContent_Declare(
		SDL2
		GIT_REPOSITORY https://github.com/libsdl-org/SDL
		GIT_TAG release-2.0.22)
	FetchContent_MakeAvailable(SDL2)
endif()
		
FetchContent_Declare(
	SDL_ttf
	GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf
	GIT_TAG release-2.20.1)			
FetchContent_MakeAvailable(SDL_ttf)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX /wd4996)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(
		-Wall 
		-Wextra 
		-pedantic 
		-Werror 
		-Wno-unused-function 	# I want unused functions for debugging purposes
		-Wno-reorder			# I want to be able to initialize class members in any order
		-Wno-class-memaccess	# I want to be able to memset() things that I know are safe to memset()
	)	
endif()

add_subdirectory(src)

