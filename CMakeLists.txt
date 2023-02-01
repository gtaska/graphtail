cmake_minimum_required (VERSION 3.19)

project(graphtail)

set(GRAPHTAIL_VERSION 1.0.0)

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

set(BUILD_SHARED_LIBS OFF)

include(FetchContent)

# ------ brotli ------
set(BROTLI_DISABLE_TESTS ON)
set(BROTLI_BUNDLED_MODE ON)

FetchContent_Declare(
	brotli
	GIT_REPOSITORY https://github.com/google/brotli
	GIT_TAG v1.0.9)
FetchContent_MakeAvailable(brotli)	

set(BROTLI_LIBRARIES_STATIC brotlidec-static)

# ------ SDL2 ------
set(SDL_TEST OFF)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_ATOMIC OFF)
set(SDL_AUDIO OFF)
set(SDL_JOYSTICK OFF)
set(SDL_HAPTIC OFF)
set(SDL_HIDAPI OFF)
set(SDL_POWER OFF)
set(SDL_THREADS OFF)
set(SDL_TIMERS OFF)
set(SDL_FILE OFF)
set(SDL_CPUINFO OFF)
set(SDL_FILESYSTEM OFF)
set(SDL_SENSOR OFF)
set(SDL_LOCALE OFF)
set(SDL_MISC OFF)

set(SDL2_DISABLE_INSTALL ON)
set(SDL2_DISABLE_UNINSTALL ON)

FetchContent_Declare(
	SDL2
	GIT_REPOSITORY https://github.com/libsdl-org/SDL
	GIT_TAG release-2.0.22)
FetchContent_MakeAvailable(SDL2)

set(SDL2_INCLUDE_DIRS "${SDL2_SOURCE_DIR}/include")
set(SDL2_LIBRARIES SDL2-static SDL2main)

# ------ SDL2_ttf ------
set(SDL2TTF_SAMPLES OFF)
set(SDL2TTF_BUILD_SHARED_LIBS OFF)
set(SDL2TTF_INSTALL OFF)

FetchContent_Declare(
	SDL_ttf
	GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf
	GIT_TAG release-2.20.1)			
FetchContent_MakeAvailable(SDL_ttf)

set(SDL_TTF_INCLUDE_DIRS "${SDL2_ttf_SOURCE_DIR}/include")
set(SDL_TTF_LIBRARIES SDL2_ttf)

# ------ Compile options ------
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

