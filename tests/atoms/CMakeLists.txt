
LINK_LIBRARIES(
   smob
   atombase
   atomspace
)

ADD_SUBDIRECTORY (atom_types)
ADD_SUBDIRECTORY (base)
ADD_SUBDIRECTORY (core)
ADD_SUBDIRECTORY (evaluation)
ADD_SUBDIRECTORY (execution)
ADD_SUBDIRECTORY (rule)
ADD_SUBDIRECTORY (join)
ADD_SUBDIRECTORY (parallel)
ADD_SUBDIRECTORY (truthvalue)
ADD_SUBDIRECTORY (value)
ADD_SUBDIRECTORY (reduct)
ADD_SUBDIRECTORY (flow)
ADD_SUBDIRECTORY (columnvec)
ADD_SUBDIRECTORY (pattern)
ADD_SUBDIRECTORY (foreign)

ADD_CXXTEST(HashUTest)
ADD_CXXTEST(RandomUTest)
ADD_CXXTEST(AlphaConvertUTest)
ADD_CXXTEST(BetaReduceUTest)
