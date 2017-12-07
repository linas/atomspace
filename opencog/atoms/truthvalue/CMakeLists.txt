ADD_LIBRARY (truthvalue
	AttentionValue.cc
	CountTruthValue.cc
	EvidenceCountTruthValue.cc
	FuzzyTruthValue.cc
	IndefiniteTruthValue.cc
	ProbabilisticTruthValue.cc
	TruthFactory.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(truthvalue opencog_atom_types)

# The truthvalue-part and truthvalue shared libs are split up this
# way to avoid a circular dependency (unresolved symbols) between
# this and the atombase shared library.
ADD_LIBRARY (truthvalue-part
	SimpleTruthValue.cc
	TruthValue.cc
)

TARGET_LINK_LIBRARIES(truthvalue
	truthvalue-part
	atombase
	${COGUTIL_LIBRARY}
)

INSTALL (TARGETS truthvalue
	DESTINATION "lib${LIB_DIR_SUFFIX}/opencog"
)

INSTALL (FILES
	AttentionValue.h
	CountTruthValue.h
	FuzzyTruthValue.h
	IndefiniteTruthValue.h
	ProbabilisticTruthValue.h
	SimpleTruthValue.h
	EvidenceCountTruthValue.h
	TruthValue.h
	DESTINATION "include/opencog/truthvalue"
)
