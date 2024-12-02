project(castle)

find_package(glfw3 CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)

add_executable(castle
	src/c_entry.cpp
	src/c_game.cpp
	src/c_input.cpp
	src/c_assets.cpp
	src/c_rendering.cpp
	src/c_camera.cpp
	src/c_audio.cpp
	src/c_modding.cpp
	src/c_animation.cpp
	src/c_main_menu.cpp
	src/c_player_ent.cpp
	src/c_enemy_ent.cpp
	src/c_world.cpp
	src/c_utils.cpp
	${CMAKE_SOURCE_DIR}/code/vendor/glad/src/glad.c

	src/c_game.h
	src/c_input.h
	src/c_assets.h
	src/c_rendering.h
	src/c_camera.h
	src/c_audio.h
	src/c_modding.h
	src/c_animation.h
	src/c_main_menu.h
	src/c_world.h
	src/c_utils.h
)

target_compile_definitions(castle PRIVATE GLFW_INCLUDE_NONE)

target_include_directories(castle PRIVATE
	include
	${CMAKE_SOURCE_DIR}/code/castle_common/include
	${CMAKE_SOURCE_DIR}/code/vendor/glad/include
)

target_link_libraries(castle PRIVATE castle_common glfw OpenAL::OpenAL)

add_dependencies(castle castle_asset_packer)

add_custom_command(TARGET castle POST_BUILD
	COMMAND $<TARGET_FILE:castle_asset_packer> ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets.dat
)
