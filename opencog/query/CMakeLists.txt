
# Build the query shlib
ADD_LIBRARY(query
	DefaultPatternMatchCB.cc
	Implicator.cc
	DefaultImplicator.cc
	InitiateSearchCB.cc
	PatternMatch.cc
	PatternMatchEngine.cc
	PatternSCM.cc
	Recognizer.cc
	Satisfier.cc
)

ADD_DEPENDENCIES(query
	opencog_atom_types
)

TARGET_LINK_LIBRARIES(query
#	lambda
#	execution
	atomspace
)

INSTALL (TARGETS query 
    EXPORT AtomSpaceTargets 
    DESTINATION "lib${LIB_DIR_SUFFIX}/opencog")

INSTALL (FILES
	BindLinkAPI.h
	DefaultImplicator.h
	DefaultPatternMatchCB.h
	Implicator.h
	InitiateSearchCB.h
	PatternMatchCallback.h
	PatternMatchEngine.h
	Satisfier.h
	DESTINATION "include/opencog/query"
)
