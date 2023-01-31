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

if(MSVC)
	set(GRAPHTAIL_FORCE_USE_SYSTEM_BROTLI_DEFAULT OFF)
	set(GRAPHTAIL_FORCE_USE_SYSTEM_SDL_DEFAULT OFF)
	set(GRAPHTAIL_FORCE_USE_SYSTEM_SDL_TTF_DEFAULT OFF)
else()
	set(GRAPHTAIL_FORCE_USE_SYSTEM_BROTLI_DEFAULT ON)
	set(GRAPHTAIL_FORCE_USE_SYSTEM_SDL_DEFAULT ON)
	set(GRAPHTAIL_FORCE_USE_SYSTEM_SDL_TTF_DEFAULT ON)
endif()

graphtail_option(GRAPHTAIL_FORCE_USE_SYSTEM_BROTLI "Force use system brotli." ${GRAPHTAIL_FORCE_USE_SYSTEM_BROTLI_DEFAULT})
graphtail_option(GRAPHTAIL_FORCE_USE_SYSTEM_SDL "Force use system SDL." ${GRAPHTAIL_FORCE_USE_SYSTEM_SDL_DEFAULT})
graphtail_option(GRAPHTAIL_FORCE_USE_SYSTEM_SDL_TTF "Force use system SDL_ttf." ${GRAPHTAIL_FORCE_USE_SYSTEM_SDL_TTF_DEFAULT})

#set(CMAKE_MODULE_PATH
#	"${CMAKE_CURRENT_SOURCE_DIR}/cmake"
#	${CMAKE_MODULE_PATH})
	
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)

if(GRAPHTAIL_FORCE_USE_SYSTEM_SDL)
	find_package(brotli REQUIRED)
else()
	set(BROTLI_DISABLE_TESTS ON)
	
	FetchContent_Declare(
		brotli
		GIT_REPOSITORY https://github.com/google/brotli
		GIT_TAG v1.0.9)
	FetchContent_MakeAvailable(brotli)	
endif()

if(GRAPHTAIL_FORCE_USE_SYSTEM_SDL)
	find_package(SDL2 REQUIRED)
else()
	set(SDL_TEST OFF)
	set(SDL_SHARED OFF)
	set(SDL_STATIC ON)
	
	FetchContent_Declare(
		SDL2
		GIT_REPOSITORY https://github.com/libsdl-org/SDL
		GIT_TAG release-2.0.22)
	FetchContent_MakeAvailable(SDL2)
endif()

if(GRAPHTAIL_FORCE_USE_SYSTEM_SDL_TTF)		
	find_package(SDL_ttf REQUIRED)
else()
	set(SDL2TTF_SAMPLES OFF)
	set(SDL2TTF_BUILD_SHARED_LIBS OFF)

	FetchContent_Declare(
		SDL_ttf
		GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf
		GIT_TAG release-2.20.1)			
	FetchContent_MakeAvailable(SDL_ttf)
endif()

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

