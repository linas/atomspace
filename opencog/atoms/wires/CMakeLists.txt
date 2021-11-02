
# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY (wires
	ChainLink.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(wires opencog_atom_types)

TARGET_LINK_LIBRARIES(wires
	atomcore
	atombase
)

INSTALL (TARGETS wires
	EXPORT AtomSpaceTargets
	DESTINATION "lib${LIB_DIR_SUFFIX}/opencog")

INSTALL (FILES
	ChainLink.h
	DESTINATION "include/opencog/atoms/wires"
)
