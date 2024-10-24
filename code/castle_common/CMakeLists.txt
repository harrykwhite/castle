project(castle_common)

add_library(castle_common STATIC
	src/cc_io.cpp
	include/castle_common/cc_io.h

	src/cc_debugging.cpp
	include/castle_common/cc_debugging.h
	
	src/cc_math.cpp
	include/castle_common/cc_math.h
	
	src/cc_assets.cpp
	include/castle_common/cc_assets.h

	src/cc_misc.cpp
	include/castle_common/cc_misc.h
)

target_include_directories(castle_common PRIVATE include)
