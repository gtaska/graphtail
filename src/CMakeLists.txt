cmake_minimum_required (VERSION 3.19)

file(GLOB C_FILES "*.cpp")
file(GLOB H_FILES "*.h")

add_executable(graphtail ${C_FILES} ${H_FILES})

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

target_include_directories(graphtail PUBLIC ${SDL2_INCLUDE_DIRS}) 
target_link_libraries(graphtail SDL2-static SDL2main)

install(TARGETS graphtail RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})