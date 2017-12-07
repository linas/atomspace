
ADD_LIBRARY (atomutils
	AtomUtils
	FindUtils
	FuzzyMatch
	FuzzyMatchBasic
	Neighbors
	TypeUtils
)

ADD_DEPENDENCIES(atomutils opencog_atom_types)

TARGET_LINK_LIBRARIES(atomutils
	atomcore
	atombase
	${COGUTIL_LIBRARY}
)

# Unify is handled seperately, to avoid circular dependencies and
# unresolved symbols between the libraries.
ADD_LIBRARY (unify
	Unify
)

TARGET_LINK_LIBRARIES(unify
	lambda
	atomcore
	atombase
	${COGUTIL_LIBRARY}
)

IF (CYGWIN)
	INSTALL (TARGETS atomutils
		DESTINATION "lib${LIB_DIR_SUFFIX}/opencog"
	)
ELSE (CYGWIN)
	INSTALL (TARGETS atomutils
		DESTINATION "lib${LIB_DIR_SUFFIX}/opencog"
	)
ENDIF (CYGWIN)

INSTALL (FILES
	AtomUtils.h
	FindUtils.h
	FollowLink.h
	ForeachChaseLink.h
	FuzzyMatch.h
	FuzzyMatchBasic.h
	ThreadSafeHandleMap.h
	Neighbors.h
	Substitutor.h
	TypeUtils.h
	Unify.h
	DESTINATION "include/opencog/atomutils"
)
