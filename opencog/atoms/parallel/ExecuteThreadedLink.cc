/*
 * opencog/atoms/parallel/ExecuteThreadedLink.cc
 *
 * Copyright (C) 2009, 2013, 2014, 2015, 2020 Linas Vepstas
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <time.h>
#include <thread>

#include <opencog/util/concurrent_queue.h>

#include <opencog/atoms/core/NumberNode.h>
#include <opencog/atoms/execution/Instantiator.h>
#include <opencog/atoms/parallel/ExecuteThreadedLink.h>
#include <opencog/atoms/value/QueueValue.h>

#include <opencog/atomspace/AtomSpace.h>

using namespace opencog;


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

/// ExecuteThreadedLink
/// Perform execution in parallel threads.
/// The general structure of this link is
///
///        ExecuteThreadedLink
///            NumberNode nthr  ; optionl; if present, number of threads.
///            SetLink
///                ExecutableAtoms...
///
/// When this link is executed, the `ExecutableAtoms...` are executed
/// in parallel, in distinct threads, with the result of the execution
/// appended to a thread-safe queue, the QueueValue.  After all of the
/// atoms have been executed, the QueueValue holding the results is
/// returned. Execution blocks until all of the threads have finished.
///
/// By default, the number of threads launched equals the number of
/// Atoms in the set. If the NumberNode is present, then the number of
/// threads is the smaller of the NumberNode and the seize of the Set.
///
/// XXX TODO: We could have a non-blocking version of this atom. We
/// could just return the QueueValue immediately; the user could check
/// to see if the queue is closed, to find out if the threads have
/// finished.

ExecuteThreadedLink::ExecuteThreadedLink(const HandleSeq&& oset, Type t)
    : Link(std::move(oset), t), _nthreads(-1), _setoff(0)
{
	if (0 == _outgoing.size())
		throw InvalidParamException(TRACE_INFO,
			"Expecting at least one argument!");

	Type nt = _outgoing[0]->get_type();
	Type st = nt;
	if (NUMBER_NODE == nt)
	{
		if (1 == _outgoing.size())
			throw InvalidParamException(TRACE_INFO,
				"Expecting a set of executable links!");
		_nthreads = std::floor(NumberNodeCast(_outgoing[0])->get_value());

		// The set link.
		_setoff = 1;
		st = _outgoing[1]->get_type();
	}

	if (SET_LINK != st)
		throw InvalidParamException(TRACE_INFO,
			"Expecting a set of executable links!");

	_nthreads = std::min(_nthreads, _outgoing[_setoff]->get_arity());
}

timespec thrc = {0,0}; // thread create
timespec inst = {0,0};
timespec exec = {0,0};
timespec push = {0,0};
timespec tryg = {0,0}; // try-get
timespec thrj = {0,0}; // join
timespec tott = {0,0};
int item = 0;
#define G ->getOutgoingAtom

static void thread_exec(AtomSpace* as, bool silent,
                        concurrent_queue<Handle>* todo,
                        QueueValuePtr qvp,
                        std::exception_ptr* returned_ex)
{
	Instantiator inst(as);
timespec start, end, diff;
	while (true)
	{
clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
		Handle h;
		if (not todo->try_get(h)) return;
clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
timespec_diff(start, end, diff);
timespec_add(tryg, diff);
int thit = item;
item++;

		// This is "identical" to what cog-execute! would do...
		try
		{
clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
			ValuePtr pap(inst.execute(h));
clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
timespec_diff(start, end, diff);
timespec_add(exec, diff);

QueueValuePtr rvp = QueueValueCast(pap);
rvp->value();
size_t sz = rvp->LinkValue::size();
if (1000 < sz) {
Handle a = h G(1);
Handle c1 = a G(0) G(1) G(0);
Handle c2 = a G(1) G(1) G(0);
Handle c3 = a G(2) G(1) G(0);
std::string name;
if (c1->get_type() == CONCEPT_NODE) name = c1->get_name();
if (c2->get_type() == CONCEPT_NODE) name = c2->get_name();
if (c3->get_type() == CONCEPT_NODE) name = c3->get_name();
printf("duuude item %d %s time=%ld %09ld\n", thit, name.c_str(),
diff.tv_sec, diff.tv_nsec);
}


			if (pap and pap->is_atom())
				pap = as->add_atom(HandleCast(pap));
clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
			qvp->push(std::move(pap));
clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
timespec_diff(start, end, diff);
timespec_add(push, diff);
		}
		catch (const std::exception& ex)
		{
			*returned_ex = std::current_exception();
			return;
		}
	}
}

ValuePtr ExecuteThreadedLink::execute(AtomSpace* as,
                                      bool silent)
{
timespec tstart, tend, tdiff;
clock_gettime(CLOCK_REALTIME, &tstart);
	// Place the work items onto a queue.
	concurrent_queue<Handle> todo_list;
	const HandleSeq& exes = _outgoing[_setoff]->getOutgoingSet();
	for (const Handle& h: exes)
		todo_list.push(h);

	// Where the results will be reported.
	QueueValuePtr qvp(createQueueValue());

	// Create a collection of joinable threads.
	std::vector<std::thread> thread_set;
	std::exception_ptr ex;

timespec start, end, diff;
	// Launch the workers
	for (size_t i=0; i<_nthreads; i++)
	{
clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
		thread_set.push_back(std::thread(&thread_exec,
			as, silent, &todo_list, qvp, &ex));
clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
timespec_diff(start, end, diff);
timespec_add(thrc, diff);
	}

clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	// Wait for it all to come together.
	for (std::thread& t : thread_set) t.join();
clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
timespec_diff(start, end, diff);
timespec_add(thrj, diff);

	// Were there any exceptions? If so, rethrow.
	if (ex) std::rethrow_exception(ex);

	qvp->close();
clock_gettime(CLOCK_REALTIME, &tend);
timespec_diff(tstart, tend, tdiff);
timespec_add(tott, tdiff);

printf("duuude did %d items\n", item);
printf("duuude thread-create %ld %09ld\n", thrc.tv_sec, thrc.tv_nsec);
printf("duuude thread-join   %ld %09ld\n", thrj.tv_sec, thrj.tv_nsec);
printf("duuude try-get       %ld %09ld\n", tryg.tv_sec, tryg.tv_nsec);
printf("duuude exec-only     %ld %09ld\n", exec.tv_sec, exec.tv_nsec);
printf("duuude push          %ld %09ld\n", push.tv_sec, push.tv_nsec);
printf("duuude total-wall    %ld %09ld\n", tott.tv_sec, tott.tv_nsec);
thrc = {0,0}; // thread create
inst = {0,0};
exec = {0,0};
push = {0,0};
tryg = {0,0}; // try-get
thrj = {0,0}; // join
tott = {0,0};
item=0;

	return qvp;
}

DEFINE_LINK_FACTORY(ExecuteThreadedLink, EXECUTE_THREADED_LINK)
