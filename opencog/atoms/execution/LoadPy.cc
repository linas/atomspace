
#include <dlfcn.h>

#include <opencog/util/exceptions.h>
#include <opencog/cython/PythonEval.h>

#include "LoadPy.h"

using namespace opencog;

/**
 * Hack around a circular shared-library dependency with python.
 * This is a hack; the cython API is mis-designed.
 */

static void* ((*pyev)(void)) = nullptr;

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
}

PythonEval& get_python_instance()
{
	if (nullptr == pyev)
		load_py_libs("libPythonEval.so");

	void *pi = (pyev)();
	return *((PythonEval*) pi);
}
