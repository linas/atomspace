/*
 * ExecSCM.cc
 *
 * Guile Scheme bindings for the execution links
 * Copyright (c) 2008, 2014, 2015 Linas Vepstas <linas@linas.org>
 */

#include <opencog/guile/SchemeModule.h>

namespace opencog {

class ExecSCM : public ModuleWrap
{
	protected:
		virtual void init(void);
		static std::vector<FunctionWrap*>* _binders;
	public:
		ExecSCM(void);
		~ExecSCM();
};

}

extern "C" {
void opencog_exec_init(void);
};


#include <cstddef>
#include <opencog/atoms/base/Link.h>
#include <opencog/atomspace/AtomSpace.h>
#include <opencog/atoms/execution/EvaluationLink.h>
#include <opencog/atoms/execution/Instantiator.h>
#include <opencog/guile/SchemeModule.h>

#include <time.h>

// ========================================================

using namespace opencog;

timespec sexe = {0,0};
int calls =0;


static void timespec_diff(struct timespec &start, struct timespec &stop,
                   struct timespec &result)
{
    if ((stop.tv_nsec - start.tv_nsec) < 0) {
        result.tv_sec = stop.tv_sec - start.tv_sec - 1;
        result.tv_nsec = stop.tv_nsec - start.tv_nsec + 1000000000L;
    } else {
        result.tv_sec = stop.tv_sec - start.tv_sec;
        result.tv_nsec = stop.tv_nsec - start.tv_nsec;
    }
}

static void timespec_add(struct timespec &acc, struct timespec &delt)
{
   acc.tv_sec += delt.tv_sec;
   acc.tv_nsec += delt.tv_nsec;
    if (1000000000L < acc.tv_nsec)
   {
      acc.tv_sec ++;
      acc.tv_nsec -= 1000000000L;
   }
}


/**
 * cog-execute! executes any/all FunctionLinks
 */
static ValuePtr ss_execute(AtomSpace* atomspace, const Handle& h)
{
timespec start, end, diff;
clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	Instantiator inst(atomspace);
	ValuePtr pap(inst.execute(h));
if (MEET_LINK == h->get_type()) {
clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
timespec_diff(start, end, diff);
timespec_add(sexe, diff);
calls++;
if (681 == calls) {
calls = 0;
printf("duude meetsy=  %ld %9ld\n", sexe.tv_sec, sexe.tv_nsec);
}
}

#if LATER
	if (pap == h)
	{
		return ValueCast(EvaluationLink::do_evaluate(atomspace, h));
	}
#endif
	if (pap and pap->is_atom())
	{
		pap = atomspace->add_atom(HandleCast(pap));
	}
	return pap;
}

/**
 * cog-evaluate! evaluates an EvaluationLink with a GPN in it.
 */
static TruthValuePtr ss_evaluate(AtomSpace* atomspace, const Handle& h)
{
	return EvaluationLink::do_evaluate(atomspace, h);
}

// ========================================================

// XXX HACK ALERT This needs to be static, in order for python to
// work correctly.  The problem is that python keeps creating and
// destroying this class, but it expects things to stick around.
// XXX FIXME: can we fix cython to not do this, already?
// Oh well. I guess that's OK, since the definition is meant to be
// for the lifetime of the process, anyway.
std::vector<FunctionWrap*>* ExecSCM::_binders = nullptr;

ExecSCM::ExecSCM(void) :
	ModuleWrap("opencog exec")
{}

/// This is called while (opencog exec) is the current module.
/// Thus, all the definitions below happen in that module.
void ExecSCM::init(void)
{
	_binders = new std::vector<FunctionWrap*>();
	_binders->push_back(new FunctionWrap(ss_execute,
	                   "cog-execute!", "exec"));

	_binders->push_back(new FunctionWrap(ss_evaluate,
	                   "cog-evaluate!", "exec"));
}

ExecSCM::~ExecSCM()
{
#if PYTHON_BUG_IS_FIXED
	for (FunctionWrap* pw : *_binders)
		delete pw;
	delete _binders;
#endif
}


void opencog_exec_init(void)
{
	static ExecSCM exy;
	exy.module_init();
}
