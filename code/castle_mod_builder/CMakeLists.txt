project(castle_mod_builder)

add_executable(castle_mod_builder
	src/cmb_main.cpp
)

target_include_directories(castle_mod_builder PRIVATE
	${CMAKE_SOURCE_DIR}/code/castle_common/include
)

target_link_libraries(castle_mod_builder PRIVATE castle_common)
