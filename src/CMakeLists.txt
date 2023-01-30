cmake_minimum_required (VERSION 3.19)

file(GLOB C_FILES "*.cpp")
file(GLOB H_FILES "*.h")

add_executable(graphtail ${C_FILES} ${H_FILES})

target_compile_features(graphtail PRIVATE cxx_std_20)
target_include_directories(graphtail PUBLIC ${SDL2_INCLUDE_DIRS}) 
target_link_libraries(graphtail SDL2-static SDL2main SDL2_ttf brotlienc-static brotlidec-static)

install(TARGETS graphtail RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})