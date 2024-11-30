project(castle_asset_packer)

find_package(Freetype REQUIRED)

add_executable(castle_asset_packer
	src/cap_main.cpp
	src/cap_textures.cpp
	src/cap_fonts.cpp
	src/cap_audio.cpp
	${CMAKE_SOURCE_DIR}/code/vendor/stb_image/src/stb_image.c

	src/cap_shared.h
	${CMAKE_SOURCE_DIR}/code/vendor/audio_file/include/AudioFile.h
)

target_include_directories(castle_asset_packer PRIVATE
	${CMAKE_SOURCE_DIR}/code/castle_common/include
	${CMAKE_SOURCE_DIR}/code/vendor/stb_image/include
	${CMAKE_SOURCE_DIR}/code/vendor/audio_file/include
)

target_link_libraries(castle_asset_packer PRIVATE Freetype::Freetype castle_common)

set_property(TARGET castle_asset_packer PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "${CMAKE_SOURCE_DIR}/assets assets.dat")
