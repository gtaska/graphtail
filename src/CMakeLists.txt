cmake_minimum_required (VERSION 3.19)

file(GLOB C_FILES "*.cpp")
file(GLOB H_FILES "*.h")

add_executable(graphtail ${C_FILES} ${H_FILES})

target_compile_features(graphtail PRIVATE cxx_std_20)
target_include_directories(graphtail PUBLIC ${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${BROTLI_INCLUDE_DIRS}) 
target_link_libraries(graphtail ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${BROTLI_LIBRARIES_STATIC})

install(TARGETS graphtail RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})