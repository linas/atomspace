
# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

INSTALL (FILES
	Alloc.h
	DESTINATION "include/opencog/alloc"
)
