
/**
 * Hack around a circular shared-library dependency with python.
 * This is a hack; the cython API is mis-designed.
 */

#include <opencog/cython/PythonEval.h>


namespace opencog {

PythonEval& get_python_instance(void);
Handle python_apply(AtomSpace*, const std::string&, const Handle&);
TruthValuePtr python_apply_tv(AtomSpace*, const std::string&, const Handle&);

};
