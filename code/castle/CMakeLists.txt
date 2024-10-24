project(castle)

find_package(glfw3 CONFIG REQUIRED)

add_executable(castle src/c_entry.cpp src/c_game.cpp src/c_game.h)

target_compile_definitions(castle PRIVATE GLFW_INCLUDE_NONE)

target_include_directories(castle PRIVATE
	${CMAKE_SOURCE_DIR}/code/castle_engine/include
	${CMAKE_SOURCE_DIR}/code/castle_common/include
	${CMAKE_SOURCE_DIR}/code/vendor/glad/include
)

target_link_libraries(castle PRIVATE glfw castle_engine castle_common)

add_dependencies(castle castle_asset_packer)
add_custom_command(TARGET castle POST_BUILD
	COMMAND $<TARGET_FILE:castle_asset_packer> ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets.dat
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/assets.dat $<TARGET_FILE_DIR:castle>/assets.dat
)
