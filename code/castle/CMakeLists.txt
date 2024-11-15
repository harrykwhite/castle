project(castle)

find_package(glfw3 CONFIG REQUIRED)

add_executable(castle
	src/c_entry.cpp

	src/c_game.cpp
	include/castle/c_game.h
	
	src/c_scene.cpp
	include/castle/c_scene.h
	
	src/c_modding.cpp
	include/castle/c_modding.h

	src/c_input.cpp
	include/castle/c_input.h

	src/c_assets.cpp
	include/castle/c_assets.h
	
	src/c_rendering.cpp
	include/castle/c_rendering.h
	
	src/c_player.cpp
	include/castle/c_player.h

	src/c_tilemap.cpp
	include/castle/c_tilemap.h

	src/c_utils.cpp
	include/castle/c_utils.h
	
	${CMAKE_SOURCE_DIR}/code/vendor/glad/src/glad.c
)

target_compile_definitions(castle PRIVATE GLFW_INCLUDE_NONE)

target_include_directories(castle PRIVATE
	include
	${CMAKE_SOURCE_DIR}/code/castle_common/include
	${CMAKE_SOURCE_DIR}/code/vendor/glad/include
)

target_link_libraries(castle PRIVATE glfw castle_common)

add_dependencies(castle castle_asset_packer)

add_custom_command(TARGET castle POST_BUILD
	COMMAND $<TARGET_FILE:castle_asset_packer> ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets.dat
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/assets.dat $<TARGET_FILE_DIR:castle>/assets.dat
)
