
#include <dlfcn.h>

#include <opencog/util/exceptions.h>
#include <opencog/cython/PythonEval.h>

#include "LoadPy.h"

using namespace opencog;

/**
 * Hack around a circular shared-library dependency bug that CMake
 * forces onto us. Its a CMake bug aka "feature", due to a MS Windows
 * bug aka "feature".  Microsoft Windows sucks. I resent that I have
 * to waste my time working around Windows bugs when I'm running
 * Linux. This is just fucked up.
 *
 * Why am I complaining?  The GNU C library linker/loader is quite
 * entirely capable of dynamically loading libraries by itself, thank
 * you very much.  So loading them by hand, like the below, is totally
 * pointless; the GNU C loader does this better than I ever could.
 * However, Windows can't do this, and CMake tries to be cross-platform,
 * and so CMake does not allow this either. As a result, I have to hack
 * around this, and dynamically load by hand.
 *
 * This is a hack; the cython API is mis-designed.
 *
 * XXX This should be fixed someday ...
 */

static void* ((*pyev)(void)) = nullptr;
static void ((*pyap)(void*, const char*, void*, void*)) = nullptr;
static void ((*pyaptv)(void*, const char*, void*, void*)) = nullptr;

static void load_py_libs(const std::string& libname)
{
	static bool is_init = false;
	if (is_init) return;
	is_init = true;

	// reset error
	dlerror();

#ifdef __APPLE__
	// Tell dyld to search runpath
	std::string withRPath("@rpath/");
	withRPath += libname;
	// Check to see if so extension is specified, replace with .dylib if it is.
	if (withRPath.substr(withRPath.size()-3, 3) == ".so")
	{
		withRPath.replace(withRPath.size()-3, 3, ".dylib");
	}
	void *dynLibrary = dlopen(withRPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#else
	void *dynLibrary = dlopen(libname.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif
	const char* dlsymError = dlerror();
	if (nullptr == dynLibrary or dlsymError)
		throw RuntimeException(TRACE_INFO,
			"Unable to load python \"%s\": %s", libname.c_str(), dlsymError);

	// Reset error
	dlerror();

	// Search for evaluator factory
	pyev = (void* ((*)(void)))
		dlsym(dynLibrary, "get_python_evaluator_instance");
	dlsymError = dlerror();
	if (dlsymError)
		throw RuntimeException(TRACE_INFO,
			"Unable to find symbol \"get_python_evaluator_instance\": %s %s",
			 libname.c_str(), dlsymError);

	pyap = (void ((*)(void*, const char*, void*, void*)))
		dlsym(dynLibrary, "cwrap_python_apply");
	dlsymError = dlerror();
	if (dlsymError)
		throw RuntimeException(TRACE_INFO,
			"Unable to find symbol \"cwrap_python_apply\": %s %s",
			 libname.c_str(), dlsymError);

	pyaptv = (void ((*)(void*, const char*, void*, void*)))
		dlsym(dynLibrary, "cwrap_python_apply_tv");
	dlsymError = dlerror();
	if (dlsymError)
		throw RuntimeException(TRACE_INFO,
			"Unable to find symbol \"cwrap_python_apply_tv\": %s %s",
			 libname.c_str(), dlsymError);
}

PythonEval& opencog::get_python_instance(void)
{
	if (nullptr == pyev)
		load_py_libs("libPythonEval.so");

	void *pi = (pyev)();
	return *((PythonEval*) pi);
}

Handle opencog::python_apply(AtomSpace* as, const std::string& expr,
                             const Handle& args)
{
	if (nullptr == pyev)
		load_py_libs("libPythonEval.so");

	Handle reth;
	(pyap)((void*) as, expr.c_str(), (void*) &args, (void*) &reth);
	return reth;
}

TruthValuePtr opencog::python_apply_tv(AtomSpace* as, const std::string& expr,
                                       const Handle& args)
{
	if (nullptr == pyev)
		load_py_libs("libPythonEval.so");

	TruthValuePtr rettv;
	(pyaptv)((void*) as, expr.c_str(), (void*) &args, (void*) &rettv);
	return rettv;
}
