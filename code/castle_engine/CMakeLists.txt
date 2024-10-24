project(castle_engine)

find_package(glfw3 CONFIG REQUIRED)

add_library(castle_engine STATIC
	src/ce_game.cpp
	include/castle_engine/ce_game.h
	
	src/ce_modding.cpp
	include/castle_engine/ce_modding.h

	src/ce_input.cpp
	include/castle_engine/ce_input.h

	src/ce_assets.cpp
	include/castle_engine/ce_assets.h
	
	src/ce_rendering.cpp
	include/castle_engine/ce_rendering.h
	
	src/ce_utils.cpp
	include/castle_engine/ce_utils.h

	${CMAKE_SOURCE_DIR}/code/vendor/glad/src/glad.c
)

target_compile_definitions(castle_engine PRIVATE GLFW_INCLUDE_NONE)

target_include_directories(castle_engine PRIVATE
	include
	${CMAKE_SOURCE_DIR}/code/castle_common/include
	${CMAKE_SOURCE_DIR}/code/vendor/glad/include
)

target_link_libraries(castle_engine PRIVATE glfw castle_common)
