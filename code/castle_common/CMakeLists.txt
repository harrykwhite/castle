project(castle_common)

add_library(castle_common STATIC
	src/cc_io.cpp
	src/cc_debugging.cpp
	src/cc_mem.cpp
	src/cc_math.cpp
	src/cc_assets.cpp
	src/cc_misc.cpp

	include/castle_common/cc_io.h
	include/castle_common/cc_debugging.h
	include/castle_common/cc_mem.h
	include/castle_common/cc_math.h
	include/castle_common/cc_assets.h
	include/castle_common/cc_misc.h
)

target_include_directories(castle_common PRIVATE include)
