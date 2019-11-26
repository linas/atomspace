/*
 * PatternMatchEngine.cc
 *
 * Copyright (C) 2008,2009,2011,2014,2015 Linas Vepstas
 *
 * Author: Linas Vepstas <linasvepstas@gmail.com>  February 2008
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

#include <opencog/util/algorithm.h>
#include <opencog/util/oc_assert.h>
#include <opencog/util/Logger.h>
#include <opencog/atoms/base/Link.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/core/FindUtils.h>
#include <opencog/atoms/pattern/PatternUtils.h>
#include <opencog/atomspace/AtomSpace.h>

#include "PatternMatchEngine.h"

using namespace opencog;

/* ======================================================== */
/**
 * Pattern Match Engine Overview
 * -----------------------------
 * The explanation of how this code *actually* works has been
 * moved to README-Algorithm. That README provides the "big-picture"
 * explanation of the rather complex interwoven code below. Read it
 * first, and refer to it when examining the main methods below.
 */
/* ======================================================== */

// The macro POPSTK has an OC_ASSERT when DEBUG is defined, so we keep
// that #define around, although it's not clear why that OC_ASSERT
// wouldn't be kept no matter what (it's not like it's gonna take up a
// lot of resources).

#define DEBUG 1
#ifdef DEBUG
#define DO_LOG(STUFF) STUFF
#else
#define DO_LOG(STUFF)
#endif


#ifdef DEBUG
static inline void log(const Handle& h)
{
	LAZY_LOG_FINE << h->to_short_string();
}

static inline void logmsg(const char * msg, const Handle& h)
{
	LAZY_LOG_FINE << msg << std::endl
	              << (h == (Atom*) nullptr ?
	                  std::string("(invalid handle)") :
	                  h->to_short_string());
}
#else
static inline void logmsg(const char * msg, const Handle& h) {}
static inline void log(const Handle& h) {}
#endif


/* ======================================================== */
/* Reset the current variable grounding to the last grounding pushed
 * onto the stack. */
#ifdef DEBUG
   #define POPSTK(stack,soln) {         \
      OC_ASSERT(not stack.empty(),      \
           "Unbalanced stack " #stack); \
      soln = stack.top();               \
      stack.pop();                      \
   }
#else
   #define POPSTK(stack,soln) {         \
      soln = stack.top();               \
      stack.pop();                      \
   }
#endif

/* ======================================================== */

/// Compare a VariableNode in the pattern to the proposed grounding.
///
/// Handle hp is from the pattern clause.  By default, the code here
/// allows a variable to be grounded by itself -- this avoids a lot
/// of second-guessing involving alpha-converted variable names,
/// which get handled at a higher layer, which has access to the
/// entire clause. (The clause_match() callback, to be specific).
///
bool PatternMatchEngine::variable_compare(const Handle& hp,
                                          const Handle& hg)
{
	// If we already have a grounding for this variable, the new
	// proposed grounding must match the existing one. Such multiple
	// groundings can occur when traversing graphs with loops in them.
	auto gnd = var_grounding.find(hp);
	if (var_grounding.end() != gnd)
		return (gnd->second == hg);

	// VariableNode had better be an actual node!
	// If it's not then we are very very confused ...
	OC_ASSERT (hp->is_node(),
	           "Expected variable to be a node, got this: %s\n",
	           hp->to_short_string().c_str());

	// Else, we have a candidate grounding for this variable.
	// The variable_match() callback may implement some tighter
	// variable check, e.g. to make sure that the grounding is
	// of some certain type.
	if (not _pmc.variable_match(hp, hg)) return false;

	// Make a record of it. Cannot record GlobNodes here; they're
	// variadic.
	if (hp->get_type() != GLOB_NODE)
	{
		DO_LOG({LAZY_LOG_FINE << "Found grounding of variable:";})
		logmsg("$$ variable:", hp);
		logmsg("$$ ground term:", hg);
		var_grounding[hp] = hg;
	}
	return true;
}

/* ======================================================== */

/// Compare an atom to itself.
///
bool PatternMatchEngine::self_compare(const PatternTermPtr& ptm)
{
	const Handle& hp = ptm->getHandle();
	if (not ptm->isQuoted()) var_grounding[hp] = hp;

	logmsg("Compare atom to itself:", hp);
	return true;
}

/* ======================================================== */

/// Compare two nodes, one in the pattern, one proposed grounding.
/// Return true if they match, else return false.
bool PatternMatchEngine::node_compare(const Handle& hp,
                                      const Handle& hg)
{
	// Call the callback to make the final determination.
	bool match = _pmc.node_match(hp, hg);
	if (match)
	{
		DO_LOG({LAZY_LOG_FINE << "Found matching nodes";})
		logmsg("# pattern:", hp);
		logmsg("# match:", hg);
		if (hp != hg) var_grounding[hp] = hg;
	}
	return match;
}

/* ======================================================== */

/// If the two links are both ordered, its enough to compare them
/// "side-by-side". Return true if they match, else return false.
/// See `tree_compare` for a general explanation.
bool PatternMatchEngine::ordered_compare(const PatternTermPtr& ptm,
                                         const Handle& hg)
{
	const PatternTermSeq& osp = ptm->getOutgoingSet();
	const HandleSeq& osg = hg->getOutgoingSet();

	// The recursion step: traverse down the tree.
	depth ++;

	// If the pattern contains no globs, then the pattern and ground
	// must match exactly, with atoms in the outgoing sets pairing up.
	// If the pattern has globs, then more complex matching is needed;
	// a glob can match one or more atoms in a row. If there are no
	// globs, and the arity is mis-matched, then perform fuzzy matching.

	bool match = true;
	const Handle &hp = ptm->getHandle();
	if (0 < _pat->globby_terms.count(hp))
	{
		match = glob_compare(osp, osg);
	}
	else
	{
		size_t osg_size = osg.size();
		size_t osp_size = osp.size();

		// If the arities are mis-matched, do a fuzzy compare instead.
		if (osp_size != osg_size)
		{
			match = _pmc.fuzzy_match(ptm->getHandle(), hg);
		}
		else
		{
			// Side-by-side recursive compare.
			for (size_t i=0; i<osp_size; i++)
			{
				if (not tree_compare(osp[i], osg[i], CALL_ORDER))
				{
					match = false;
					break;
				}
			}
		}
	}

	depth --;
	DO_LOG({LAZY_LOG_FINE << "ordered_compare match?=" << match;})

	if (not match)
	{
		_pmc.post_link_mismatch(hp, hg);
		return false;
	}

	// If we've found a grounding, lets see if the
	// post-match callback likes this grounding.
	match = _pmc.post_link_match(hp, hg);
	if (not match) return false;

	// If we've found a grounding, record it.
	record_grounding(ptm, hg);

	return true;
}

/* ======================================================== */

/// Compare a ChoiceLink in the pattern to the proposed grounding.
/// The term `ptm` points at the ChoiceLink.
///
/// CHOICE_LINK's are multiple-choice links. As long as we can
/// can match one of the sub-expressions of the ChoiceLink, then
/// the ChoiceLink as a whole can be considered to be grounded.
///
bool PatternMatchEngine::choice_compare(const PatternTermPtr& ptm,
                                        const Handle& hg)
{
	const Handle& hp = ptm->getHandle();
	PatternTermSeq osp = ptm->getOutgoingSet();

	// _choice_state lets use resume where we last left off.
	size_t iend = osp.size();
	size_t icurr = curr_choice(ptm, hg);

	DO_LOG({LAZY_LOG_FINE << "tree_comp resume choice search at " << icurr
	              << " of " << iend << " of term=" << ptm->to_string()
	              << ", choose_next=" << _choose_next;})

	// XXX This is almost surely wrong... if there are two
	// nested choice links, then this will hog the steps,
	// and the deeper choice will fail.
	if (_choose_next)
	{
		icurr++;
		_choose_next = false; // we are taking a step, so clear the flag.
	}

	while (icurr<iend)
	{
		solution_push();
		const PatternTermPtr& hop = osp[icurr];

		DO_LOG({LAZY_LOG_FINE << "tree_comp or_link choice " << icurr
		              << " of " << iend;})

		bool match = tree_compare(hop, hg, CALL_CHOICE);
		if (match)
		{
			// If we've found a grounding, lets see if the
			// post-match callback likes this grounding.
			match = _pmc.post_link_match(hp, hg);
			if (match)
			{
				// Even the stack, *without* erasing the discovered grounding.
				solution_drop();

				// If the grounding is accepted, record it.
				record_grounding(ptm, hg);

				_choice_state[GndChoice(ptm, hg)] = icurr;
				return true;
			}
		}
		else
		{
			_pmc.post_link_mismatch(hp, hg);
		}
		solution_pop();
		_choose_next = false; // we are taking a step, so clear the flag.
		icurr++;
	}

	// If we are here, we've explored all the possibilities already
	_choice_state.erase(GndChoice(ptm, hg));
	return false;
}

/// Return the current choice state for the given pattern & ground
/// combination.
size_t PatternMatchEngine::curr_choice(const PatternTermPtr& ptm,
                                       const Handle& hg)
{
	auto cs = _choice_state.find(GndChoice(ptm, hg));
	if (_choice_state.end() == cs)
	{
		_choose_next = false;
		return 0;
	}
	return cs->second;
}

bool PatternMatchEngine::have_choice(const PatternTermPtr& ptm,
                                     const Handle& hg)
{
	return 0 < _choice_state.count(GndChoice(ptm, hg));
}

/* ======================================================== */
static int facto (int n) { return (n==1)? 1 : n * facto(n-1); };

/// Unordered link comparison
///
/// Compare two unordered links, side by side. In some ways, this is
/// similar to the ordered link compare: for a given, fixed permutation
/// of the unordered link, the compare is side by side.  However, if
/// that compare fails, the next permutation must then be tried, until
/// a match is found or all permutations are exhausted.  But there's a
/// problem: if there are multiple, nested unordered links, or if they
/// are peers (siblings) in the tree, then if one takes a step, the
/// other must not. Coordinating this is difficult, and requires a long
/// explanation. So here goes:
///
/*****************************************************************

How do unordered links work?
----------------------------
This is complicated, so we write it out.  When ascending from below (i.e.
from do_term_up()), unordered links may be found in two different
places: The parent term may be unordered, or the parent link may hold
another link (a sibling to us) that is unordered. Traversal needs to
handle both cases.  Thus, the upwards-movement methods (do_term_up(),
explore_up_branches(), etc.) are incapable of discovering unordered links,
as they cannot "see" the siblings.  Siblings can only be found during
tree-compare, moving downwards.  Thus, tree_compare must do a lot of
heavy lifting.

When comparing trees downwards, we have two situations we may be in:
we may be asked to compare things exactly like last time, or we may be
asked to try out the next permutation. We need to report back two bits
of state: whether or not we found a match, and whether or not we have
more possibilities to explore. Our behavior is thus controlled by this
third bit, as well as what happened below us.

The correct actions to take are best explored by truth tables: the
settings of the _take_step and _have_more flags on entry, and what to
do after running tree_compare downwards. These are handled by two
truth tables.

The topmost routine to call tree_compare must *always* set _have_more=F
and _take_step=T before calling tree_compare.  This will cause
tree_compare to advance to the next matching permutation, or to run until
all permutations are exhausted, and no match was found.


Flag settings upon entry
------------------------

     take  have
case step  more    Comments / Action to be Taken
---- ----  ----    -----------------------------
  A    T    T     Impossible, Who set have_more? Who set take_step?
  B    T    F     Normal entry: we are the first unordered link to be
                  hit during downward traversal. Proceed using the truth
                  table below.
  C    F    T     We are not the first unorder. Someone ran before us.
                  If we are in the very first permutation, (i.e. we are
                    entering for the first time) we must call downwards.
                    If this returns a mismatch, we must step until we
                    find a match, or until we exhaust all permutations.
                    See next table for what to return: we return cases
                    5, 7 or 8.
                  If we are not the first permutation, we could just
                    return T, because that is what we did last time.
                    i.e. we are told to not take a step, so we don't,
                    and we know a-priori last time were were here, we
                    must have returned a match.  Thus, we can return
                    case 5 below.  We cannot return case 7 because we
                    can't know what std::next_perm returns.
                    (See however, footnote below).
                  If we hold an evaluatable, we must call down.
  D    F    F     Perform same as C above.

Footnote: case C: Well, the reasoning there is almost right, but not
quite. If the unordered link contains a variable, and it is also not in
the direct line of exploration (i.e. its grounding is NOT recorded)
then its truthiness holds only for a grounding that no longer exists.
Thus, for case C, it is safer to always check.

However, by the above reasoning: if the grounding wasn't recorded
(because the link is not in the recursion path) then the permutation
should not be recorded either. It should start with a permutation from
nothing.  XXX FIXME ... except we have no test case that illustrates
this failure mode.  It would require a peer unordered link that takes
a different order when the parent changes. Perhaps unordered links
nested inside a ChoiceLink would trigger this?

If case B was encountered on entry, we call downwards ourselves, and
then report back, according to the truth table below.

     returned result flags
     take  have   got
case step  more  match    Comments / Action to be Taken
---- ----  ----  ----     ------------------------------
 1     T    T      T    Impossible, error: who set have_more w/o
                          taking step??
 2     T    T      F    ditto
 3     T    F      T    We have no unordered links below us; we are at
                        the bottom.  If there had been unordered links,
                        they would have cleared the take_step flag. The
                        match we detected is the same match the last
                        time we were here. So we take a step, call
                        down again, and keep stepping until there is a
                        match or until all permutations are exhausted.
                          If match, we return: take_step=F,
                            have_more = result of std::next_perm
                            (we return case 5 or 7)
                          If exhaust, we return take_step=F, have_more=F
                            (we return case 8)

 4     T    F      F    If we are not holding any evaluatable links,
                        then this is impossible, as last time we were
                        here, we returned T.  If we are holding
                        evaluatable links, then one of them changed its
                        mind. Oh well. Take a step, proceed as in case 3.

 5     F    T      T    Someone below us took a step. Do nothing.
                        Return case 5 flags.
 6     F    T      F    Impossible; link that took the step should have
                        stepped until match or exhaustion.
 7     F    F      T    Unusual; its the last match for a lower unordered
                        link. We report the match. We do not take a
                        step; we do set the have_more flag to indicate
                        that we ourselves still have more.  i.e. we
                        return case 5 flags.
 8     F    F      F    Typical; a lower unordered link ran to exhaustion,
                        and got nada.  So *we* take a step, and call
                        downwards again. We keep going till match or till
                        exhaustion. If there's a match, we expect to see
                        the case 5 flags, so we halt and return.  If we
                        exhaust, we return case 8.

The above assumes that the curr_perm() method always returns either
the current permutation, or it returns a fresh permutation. If it returned
a fresh permutation, this counts as "taking a step", so we need to know
this.

******************************************************************/

bool PatternMatchEngine::unorder_compare(const PatternTermPtr& ptm,
                                         const Handle& hg)
{
	const Handle& hp = ptm->getHandle();
	const HandleSeq& osg = hg->getOutgoingSet();
	PatternTermSeq osp = ptm->getOutgoingSet();
	size_t arity = osp.size();
	bool has_glob = (0 < _pat->globby_holders.count(hp));

	// They've got to be the same size, at the least!
	// unless there are globs in the pattern
	if (osg.size() != arity and not has_glob)
		return _pmc.fuzzy_match(hp, hg);

	// Test for case A, described above.
	OC_ASSERT (nullptr == _perm_take_step or nullptr == _perm_have_more,
	           "Impossible situation! BUG!");

	if (nullptr == _perm_first_term) _perm_first_term = ptm;

#if 0
	// If we are coming up from below, through this particular
	// ptm, we must not take any steps, or reset it.
	if (_perm_reset)
	{
		_perm_reset = false;
		_perm_state.erase(Unorder(ptm, hg));
		_perm_count.erase(Unorder(ptm, hg));
	}
#endif

	// _perm_state lets use resume where we last left off.
	Permutation mutation = curr_perm(ptm, hg);
	bool do_wrap = (nullptr != _perm_take_step);
	PatternTermPtr save_have_more = _perm_have_more;

	// Cases C and D fall through.
	// If we are here, we've got possibilities to explore.
#ifdef DEBUG
	int num_perms = 0;
	if (logger().is_fine_enabled())
	{
		num_perms = facto(mutation.size());
		logger().fine("tree_comp RESUME unordered search at %d of %d of term=%s "
		              "take_step=%d have_more=%d\n",
		              _perm_count[Unorder(ptm, hg)] + 1, num_perms,
		              ptm->to_string().c_str(),
		              nullptr != _perm_take_step,
		              nullptr != _perm_have_more);
	}
#endif
	do
	{
		DO_LOG({LAZY_LOG_FINE << "tree_comp explore unordered perm "
		              << _perm_count[Unorder(ptm, hg)] +1 << " of " << num_perms
		              << " of term=" << ptm->to_string();})
		solution_push();
		bool match = true;

		if (has_glob)
		{
			// Each glob comparison steps the glob state forwards.
			// Each different permutation has to start with the
			// same glob state as before. So save and restore state.
			std::map<GlobPair, GlobState> saved_glob_state = _glob_state;
			match = glob_compare(mutation, osg);
			_glob_state = saved_glob_state;
		}
		else
		{
			for (size_t i=0; i<arity; i++)
			{
				if (not tree_compare(mutation[i], osg[i], CALL_UNORDER))
				{
					match = false;
					break;
				}
			}
		}

		// Check for cases 1&2 of description above.
		// These flags might have been (mis-)set in the
		// call to `tree_compare()` immediately above.
		OC_ASSERT(nullptr == _perm_take_step or nullptr ==_perm_have_more,
		          "This shouldn't happen. Impossible situation! BUG!");

		_perm_reset = false;

		// Handle cases 3&4 of the description above. That is, the
		// `tree_compare()` above did not take a step, so we will.
		// The `tree_compare()` should have reported exactly the
		// same results as last time (i.e. a match or eval) and so
		// neither a `post_link_match()` nor a `post_link_mismatch()`
		// should be reported.
		if (ptm == _perm_take_step and nullptr == _perm_have_more)
		{
			OC_ASSERT(match or (0 < _pat->evaluatable_holders.count(hp)),
			          "Impossible: should have matched!");
			goto take_next_step;
		}

		if (ptm != _perm_take_step and nullptr != _perm_take_step)
		{
			// OC_ASSERT(match or (0 < _pat->evaluatable_holders.count(hp)),
			//          "Impossible: should have matched!");
			return match;
		}

		// If we are here, then _take_step is false, because
		// cases 1,2,3,4 already handled above.
		// Well, actually, this can happen, if we are not careful
		// to manage the _have_more flag on a stack.
//		OC_ASSERT(match or not _perm_have_more or 1==num_perms,
//		          "Impossible: case 6 happened!");

		if (match)
		{
			// If we've found a grounding, lets see if the
			// post-match callback likes this grounding.
			match = _pmc.post_link_match(hp, hg);
			if (match)
			{
				// Even the stack, *without* erasing the discovered grounding.
				solution_drop();

				// If the grounding is accepted, record it.
				record_grounding(ptm, hg);

				// Handle case 5&7 of description above.
				DO_LOG({LAZY_LOG_FINE << "Good permutation "
				              << _perm_count[Unorder(ptm, hg)] + 1
				              << " of " << num_perms
				              << " for term=" << ptm->to_string();})
				_perm_state[Unorder(ptm, hg)] = mutation;

				if (nullptr == _perm_have_more or
					(nullptr != save_have_more and
					   save_have_more == _perm_have_more))
				{
					_perm_have_more = ptm;
				}
				_perm_take_step = nullptr;
				_perm_latest_term = ptm;
				_perm_reset = false;
				return true;
			}
		}
		else
		{
			_pmc.post_link_mismatch(hp, hg);
		}

		// If we are here, we are handling case 8.
		DO_LOG({LAZY_LOG_FINE << "Bad permutation "
		              << _perm_count[Unorder(ptm, hg)] + 1
		              << " of " << num_perms
		              << " for term=" << ptm->to_string();})

take_next_step:
		_perm_take_step = nullptr; // we are taking the step, so clear the flag.
		_perm_have_more = nullptr; // start with a clean slate...
		_perm_reset = true;      // reset perms on lower links, too.
		solution_pop();
		if (logger().is_fine_enabled())
			_perm_count[Unorder(ptm, hg)] ++;
	} while (std::next_permutation(mutation.begin(), mutation.end(),
	         std::less<PatternTermPtr>()));

	// If we are here, we've explored all the possibilities already
	DO_LOG({LAZY_LOG_FINE << "Exhausted all permutations of term="
	             << ptm->to_string() << " do_wrap=" << do_wrap;})
	_perm_state.erase(Unorder(ptm, hg));
	_perm_count.erase(Unorder(ptm, hg));
	// _perm_take_step = nullptr;
	_perm_have_more = save_have_more;
	_perm_reset = false;
	_perm_latest_term = ptm;

#if 0
	// Implement an "odometer", for iterating on other unordered
	// links that might occur in series with this one. That is,
	// wrap around the permutation set for this link, while also
	// advancing the next link by one (setting _take_step causes
	// the next link to advance).
	if (_perm_have_odometer and do_wrap)
	{
		bool match = unorder_compare(ptm, hg);
		if (not match) return false;
		_perm_latest_wrap = ptm;
		_perm_have_more = nullptr;
		_perm_take_step = ptm;
		return true;
	}
#endif
	return false;
}


/// Detect if the PatternTermPtr is a clause, so that we know
/// that it is time to stop moving upwards. When quotations are
/// being used, we  may have already moved past the top of the
/// clause!! ... which seems strange to me, but that is how
/// quotations work.
bool PatternMatchEngine::term_is_a_clause(const PatternTermPtr& ptm,
                                          const Handle& clause)
{
	return ptm->getHandle() == clause
		or (Quotation::is_quotation_type(clause->get_type())
		    and ptm->getHandle() == clause->getOutgoingAtom(0));
}

/// Return the saved unordered-link permutation for this
/// particular point in the tree comparison (i.e. for the
/// particular unordered link hp in the pattern.)
PatternMatchEngine::Permutation
PatternMatchEngine::curr_perm(const PatternTermPtr& ptm,
                              const Handle& hg)
{
	auto ps = _perm_state.find(Unorder(ptm, hg));
	if (_perm_state.end() == ps)
	{
		DO_LOG({LAZY_LOG_FINE << "tree_comp FRESH START unordered term="
		              << ptm->to_string();})
		Permutation perm = ptm->getOutgoingSet();
		// Sort into explict std::less<PatternTermPtr>() order, as
		// otherwise std::next_permutation() will miss some perms.
		sort(perm.begin(), perm.end(), std::less<PatternTermPtr>());
		_perm_take_step = nullptr;
		return perm;
	}
	return ps->second;
}

/// Return true if there are more permutations to explore.
/// Else return false.
bool PatternMatchEngine::have_perm(const PatternTermPtr& ptm,
                                   const Handle& hg)
{
	if (_perm_state.end() == _perm_state.find(Unorder(ptm, hg)))
		return false;
	return true;
}

void PatternMatchEngine::perm_push(void)
{
	perm_stack.push(_perm_state);
	if (logger().is_fine_enabled())
		_perm_count_stack.push(_perm_count);
}

void PatternMatchEngine::perm_pop(void)
{
	POPSTK(perm_stack, _perm_state);
	if (logger().is_fine_enabled())
		POPSTK(_perm_count_stack, _perm_count);
}

/* ======================================================== */

/// Compare the outgoing sets of two trees side-by-side, where
/// the pattern contains at least one GlobNode.
bool PatternMatchEngine::glob_compare(const PatternTermSeq& osp,
                                      const HandleSeq& osg)
{
	bool match = true;
	GlobPair gp = {osp, osg};
	size_t osp_size = osp.size();
	size_t osg_size = osg.size();

	size_t ip = 0;
	size_t jg = 0;

	GlobGrd glob_grd;
	GlobPosStack glob_pos_stack;

	// Common things that need to be done when backtracking.
	bool backtracking = false;
	bool cannot_backtrack_anymore = false;
	auto backtrack = [&](bool is_glob)
	{
		backtracking = true;

		// If we are looking at a glob right now and fail
		// to ground it, pop the stack, go back to the
		// previous one and try again.
		if (is_glob)
		{
			// Erase the grounding record of the glob before
			// popping it out from the stack.
			glob_grd.erase(glob_pos_stack.top().first);

			glob_pos_stack.pop();
			_glob_state[gp] = {glob_grd, glob_pos_stack};
		}

		// See where the previous glob is and try again
		// from there.
		if (0 == glob_pos_stack.size())
			cannot_backtrack_anymore = true;
		else
		{
			ip = glob_pos_stack.top().second.first;
			jg = glob_pos_stack.top().second.second;

			solution_pop();
		}
	};

	// Common things that need to be done when a match
	// is found for a glob.
	auto record_match = [&](const PatternTermPtr& glob,
	                        const HandleSeq& glob_seq)
	{
		solution_push();

		glob_grd[glob] = glob_seq.size();
		_glob_state[gp] = {glob_grd, glob_pos_stack};

		Handle glp(createLink(glob_seq, LIST_LINK));
		var_grounding[glob->getHandle()] = glp;

		DO_LOG({LAZY_LOG_FINE << "Found grounding of glob:";})
		logmsg("$$ glob:", glob->getHandle());
		logmsg("$$ ground term:", glp);
	};

	// Common things needed to be done when it's not a match.
	auto mismatch = [&]()
	{
		match = false;
		_glob_state.erase(gp);
	};

	// Resume the matching from a previous state.
	// i.e. we had successfully grounded osp to osg, but it
	// turns out the groundings do not satisfy some other terms
	// in the same pattern, so we try again and see if the globs
	// in osp can be grounded differently.
	auto r = _glob_state.find(gp);
	if (r != _glob_state.end())
	{
		backtracking = true;

		solution_pop();
		glob_grd = r->second.first;
		glob_pos_stack = r->second.second;
		ip = glob_pos_stack.top().second.first;
		jg = glob_pos_stack.top().second.second;
	}

	while (ip<osp_size)
	{
		// Reject if no more backtracking is possible.
		if (cannot_backtrack_anymore)
		{
			mismatch();
			break;
		}

		const Handle& ohp(osp[ip]->getHandle());
		Type ptype = ohp->get_type();

		if (GLOB_NODE == ptype)
		{
			HandleSeq glob_seq;
			PatternTermPtr glob(osp[ip]);

			// A glob may appear more than once in the pattern,
			// so check if that's the case. If we have already
			// grounded it previously, make sure the grounding
			// here is consistent with the earlier grounding.
			auto vg = var_grounding.find(ohp);
			if (not backtracking and vg != var_grounding.end())
			{
				bool no_match = false;

				// The grounding of a glob is wrapped in a ListLink,
				// so compare the outgoing set of it.
				for (const Handle& h : vg->second->getOutgoingSet())
				{
					if (jg >= osg_size or h != osg[jg])
					{
						no_match = true;
						break;
					}
					jg++;
				}

				// Backtrack if the previous grounding does not fit here.
				if (no_match) backtrack(false);
				// Otherwise, it's a match, move on.
				else ip++;

				continue;
			}

			// No need to push to stack if we are backtracking.
			if (backtracking)
			{
				// Reset the flag, so that the next glob will be
				// pushed to the stack.
				backtracking = false;
			}
			else
			{
				// XXX why are we not doing any checks to see if the
				// grounding meets the variable constraints?
				glob_pos_stack.push({glob, {ip, jg}});
				_glob_state[gp] = {glob_grd, glob_pos_stack};
			}

			// First of all, see if we have seen this glob in
			// previous iterations.  Huh ??? Why???
			size_t last_grd = SIZE_MAX;
			auto gi = glob_grd.find(glob);
			if (gi != glob_grd.end())
			{
				last_grd = gi->second;
			}

			// If the lower bound of the interval is zero, the glob
			// can be grounded to nothing.
			if (_variables->is_lower_bound(ohp, 0))
			{
				// Try again, find another glob that can be grounded
				// in a different way. (we are probably resuming the
				// search from a previous state)
				if (0 == last_grd)
				{
					backtrack(true);
					continue;
				}

				// On the other hand, if we failed to ground this glob
				// in the previous iteration, just let it ground to
				// nothing (as long as it is not the last one in osp),
				// and we are done with it.
				if (1 == last_grd and ip+1 < osp_size)
				{
					record_match(glob, glob_seq);
					ip++;
					continue;
				}

				// If we have already gone through all the atoms of
				// the candidate at this point, we are done.
				if (jg >= osg_size)
				{
					record_match(glob, glob_seq);
					ip++;
					continue;
				}

				// Just in case, if the upper bound is zero...
				// XXX Huh ???
				if (not _variables->is_upper_bound(ohp, 1))
				{
					record_match(glob, glob_seq);
					ip++;
					continue;
				}
			}

			// If we are here, that means we have to ground the glob to
			// at least one atom.

			// Try again if we have already gone through everything in osg.
			if (jg >= osg_size)
			{
				backtrack(true);
				continue;
			}

			// Try to match as many atoms as possible.
			bool tc;
			do
			{
				tc = tree_compare(glob, osg[jg], CALL_GLOB);
				if (tc)
				{
					// Can't match more than it did last time.
					if (glob_seq.size()+1 >= last_grd)
					{
						jg--;
						break;
					}

					// Can't exceed the upper bound.
					if (not _variables->is_upper_bound(ohp, glob_seq.size()+1))
					{
						jg--;
						break;
					}

					glob_seq.push_back(osg[jg]);

					// See if we can match the next one.
					jg++;
				}
				// Can't match more, e.g. a type mis-match
				else jg--;
			} while (tc and jg<osg_size);

			// Try again if we can't ground the glob after all.
			if (0 == glob_seq.size())
			{
				backtrack(true);
				continue;
			}

			// Try again if we can't ground enough atoms to satisfy
			// the lower bound restriction.
			if (not _variables->is_lower_bound(ohp, glob_seq.size()))
			{
				backtrack(true);
				continue;
			}

			// Try again if there is no more osp to be explored but
			// we haven't finished osg yet.
			if (ip+1 == osp_size and jg+1 < osg_size)
			{
				backtrack(true);
				continue;
			}

			// If we are here, we've got a match; record the glob.
			record_match(glob, glob_seq);

			// Try to match another one.
			ip++; jg++;
		}
		else
		{
			// If we are here, we are not comparing to a glob.

			// Try again if we have already gone through all the
			// atoms in osg.
			if (jg >= osg_size)
			{
				backtrack(false);
				continue;
			}

			// Try again if we reached the end of osp, but there
			// are two or more atoms to be matched in osg (because
			// maybe we can match one atom with the final atom of the
			// pattern, but we certainly cannot match two or more.)
			if (ip+1 == osp_size and jg+1 < osg_size)
			{
				backtrack(false);
				continue;
			}

			// Try again if this pair is not a match.
			if (not tree_compare(osp[ip], osg[jg], CALL_ORDER))
			{
				backtrack(false);
				continue;
			}

			ip++; jg++;
		}
	}

	return match;
}

/* ======================================================== */
/**
 * tree_compare compares two trees, side-by-side.
 *
 * Compare two incidence trees, side-by-side.  The incidence tree is
 * given by following the "outgoing set" of the links appearing in the
 * tree.  The incidence tree is the so-called "Levi graph" of the
 * hypergraph.  The first arg should be a handle to a term in the
 * pattern, while the second arg is a handle to a candidate grounding.
 * The pattern (template) clause is compared to the candidate grounding,
 * returning true if there is a match, else return false.
 *
 * The comparison is recursive, so this method calls itself on each
 * subtree (term) of the template term, performing comparisons until a
 * match is found (or not found).
 *
 * The pattern clause may contain quotes (QuoteLinks), which signify
 * that what follows must be treated as a literal (constant), rather
 * than being interpreted.  Thus, quotes can be used to search for
 * expressions containing variables (since a quoted variable is no
 * longer a variable, but a constant).  Quotes can also be used to
 * search for GroundedPredicateNodes (since a quoted GPN will be
 * treated as a constant, and not as a function).  Quotes can be nested,
 * only the first quote is used to escape into the literal context,
 * and so quotes can be used to search for expressions containing
 * quotes.  It is assumed that the QuoteLink has an arity of one, as
 * its quite unclear what an arity of more than one could ever mean.
 *
 * This method has side effects. The main one is to insert variable
 * groundings and term groundings into `var_grounding` when grounded
 * variables and grounded terms are discovered in the pattern. (A term
 * is gounded when all variables in it are grounded). This is done
 * progressively, so that earlier groundings will be recorded even if
 * later ones fail. Thus, in order to use this method safely, the caller
 * must make a temp copy of `var_grounding`, and restore the temp if
 * there is no match.
 */
bool PatternMatchEngine::tree_compare(const PatternTermPtr& ptm,
                                      const Handle& hg,
                                      Caller caller)
{
	const Handle& hp = ptm->getHandle();

	// Do we already have a grounding for this? If we do, and the
	// proposed grounding is the same as before, then there is
	// nothing more to do.
	auto gnd = var_grounding.find(hp);
	if (gnd != var_grounding.end()) return (gnd->second == hg);

	Type tp = hp->get_type();

	// If the pattern is a DefinedSchemaNode, we need to substitute
	// its definition. XXX TODO.
	if (DEFINED_SCHEMA_NODE == tp)
		throw RuntimeException(TRACE_INFO, "Not implemented!!");

	// Handle hp is from the pattern clause, and it might be one
	// of the bound variables. If so, then declare a match.
	if (not ptm->isQuoted())
	{
		if (_variables->varset.end() != _variables->varset.find(hp))
			return variable_compare(hp, hg);

		// Report other variables that might be found.
		if (VARIABLE_NODE == tp)
			return _pmc.scope_match(hp, hg);
	}

	// If they're the same atom, then clearly they match.
	//
	// If the pattern contains atoms that are evaluatable i.e. GPN's
	// then we must fall through, and let the tree comp mechanism
	// find and evaluate them. That's for two reasons: (1) because
	// evaluation may have side-effects (e.g. send a message) and
	// (2) evaluation may depend on external state. These are
	// typically used to implement behavior trees, e.g SequenceUTest
	if ((hp == hg) and not is_evaluatable(hp))
		return self_compare(ptm);

	// If both are nodes, compare them as such.
	if (hp->is_node() and hg->is_node())
		return node_compare(hp, hg);

	// CHOICE_LINK's are multiple-choice links. As long as we can
	// can match one of the sub-expressions of the ChoiceLink, then
	// the ChoiceLink as a whole can be considered to be grounded.
	// Note, we must do this before the fuzzy_match below, because
	// hg might be a node (i.e. we compare a choice of nodes to one
	// node).
	if (CHOICE_LINK == tp)
		return choice_compare(ptm, hg);

	// If they're not both links, then it is clearly a mismatch.
	if (not (hp->is_link() and hg->is_link())) return _pmc.fuzzy_match(hp, hg);

	// Let the callback perform basic checking.
	bool match = _pmc.link_match(ptm, hg);
	if (not match) return false;

	DO_LOG({LAZY_LOG_FINE << "depth=" << depth;})
	logmsg("tree_compare:", hp);
	logmsg("to:", hg);

	// If the two links are both ordered, its enough to compare
	// them "side-by-side".
	if (2 > hp->get_arity() or not _nameserver.isA(tp, UNORDERED_LINK))
		return ordered_compare(ptm, hg);

	// If we are here, we are dealing with an unordered link.
	return unorder_compare(ptm, hg);
}

/* ======================================================== */

/*
 * The input pattern may contain many repeated sub-patterns. For example:
 *
 * ImplicationLink
 *   UnorderedLink
 *     VariableNode "$x"
 *     ConceptNode "this one"
 *   UnorderedLink
 *     VariableNode "$x"
 *     ConceptNode "this one"
 *
 * Suppose that we start searching the clause from VariableNode "$x" that
 * occures twice in the pattern under UnorderedLink. While we traverse
 * the pattern recursively we need to keep current state of permutations
 * of UnorderedLinks. We do not know which permutation will match. It may
 * be different permutation for each occurence of UnorderedLink-s.
 * This is the reason why we use PatternTerm pointers instead of atom Handles
 * while traversing pattern tree. We need to keep permutation states for
 * each term pointer separately.
 *
 * Next suppose our joining atom repeats in several sub-branches of a single
 * ChoiceLink. For example:
 *
 * ChoiceLink
 *   UnorderedLink
 *     VariableNode "$x"
 *     ConceptNode "this one"
 *   UnorderedLink
 *     VariableNode "$x"
 *     ConceptNode "this one"
 *
 * We start pattern exploration for each occurence of joining atom. This
 * is required, due to the pruning done in explore_choice_branches()
 * when the first match is found. XXX This may need to be refactored.
 * For now, we iterate over all pattern terms associated with a given
 * atom handle.
 */
bool PatternMatchEngine::explore_term_branches(const Handle& term,
                                               const Handle& hg,
                                               const Handle& clause)
{
	// The given term may appear in the clause in more than one place.
	// Each distinct location should be explored separately.
	auto pl = _pat->connected_terms_map.find({term, clause});
	OC_ASSERT(_pat->connected_terms_map.end() != pl, "Internal error");

	// Check if the pattern has globs in it.
	bool has_glob = (0 < _pat->globby_holders.count(term));

	for (const PatternTermPtr &ptm : pl->second)
	{
		DO_LOG({LAZY_LOG_FINE << "Begin exploring term: " << ptm->to_string();})
		bool found;
		if (has_glob)
			found = explore_glob_branches(ptm, hg, clause);
		else
			found = explore_odometer(ptm, hg, clause);

		DO_LOG({LAZY_LOG_FINE << "Finished exploring term: "
		                      << ptm->to_string()
		                      << " found=" << found; })
		if (found) return true;
	}
	return false;
}

/// explore_up_branches -- look for groundings for the given term.
///
/// The argument passed to this function is a term that needs to be
/// grounded. One of this term's children has already been grounded:
/// the term's child is in `hp`, and the corresponding grounding is
/// in `hg`.  Thus, if the argument is going to be grounded, it will
/// be grounded by some atom in the incoming set of `hg`. Viz, we are
/// walking upwards in these trees, in lockstep.
///
/// This method wraps the major branch-point of the entire pattern
/// matching process. Each element of the incoming set is the start of
/// a different possible branch to be explored; each one might yeild
/// a grounding. Thus, when backtracking, after a failed grounding in
/// one branch, we backtrack to here, and try another branch. When
/// backtracking, all state must be popped and pushed again, to enter
/// the new branch. We don't pushd & pop here, we push-n-pop in the
/// explore_unordered_branches() method.
///
/// This method is part of a recursive chain that only terminates
/// when a grounding for *the entire pattern* was found (and the
/// grounding was accepted) or if all possibilities were exhaustively
/// explored.  Thus, this returns true only if entire pattern was
/// grounded.
///
bool PatternMatchEngine::explore_up_branches(const PatternTermPtr& ptm,
                                             const Handle& hg,
                                             const Handle& clause)
{
	// Check if the pattern has globs in it.
	if (0 < _pat->globby_holders.count(ptm->getHandle()))
		return explore_upglob_branches(ptm, hg, clause);
	return explore_upvar_branches(ptm, hg, clause);
}

/// Same as explore_up_branches(), handles the case where `ptm`
/// is specifying a VariableNode only. This is a straighforward
/// loop over the incoming set, and nothing more.
bool PatternMatchEngine::explore_upvar_branches(const PatternTermPtr& ptm,
                                             const Handle& hg,
                                             const Handle& clause_root)
{
	// Move up the solution graph, looking for a match.
	IncomingSet iset = _pmc.get_incoming_set(hg);
	size_t sz = iset.size();
	DO_LOG({LAZY_LOG_FINE << "Looking upward at term = "
	              << ptm->getHandle()->to_string()
	              << "The grounded pivot point " << hg->to_string()
	              << " has " << sz << " branches";})

	bool found = false;
	for (size_t i = 0; i < sz; i++)
	{
		DO_LOG({LAZY_LOG_FINE << "Try upward branch " << i+1 << " of " << sz
		              << " at term=" << ptm->to_string()
		              << " propose=" << iset[i]->to_string();})

		PatternTermPtr save_more = _perm_have_more;
		PatternTermPtr save_take = _perm_take_step;
		found = explore_type_branches(ptm, Handle(iset[i]), clause_root);
		_perm_have_more = save_more;
		_perm_take_step = save_take;
		if (found) break;
	}

	DO_LOG({LAZY_LOG_FINE << "Found upward soln = " << found;})
	return found;
}

/// Same as explore_up_branches(), handles the case where `ptm`
/// has a GlobNode in it. In this case, we need to loop over the
/// inconoming, just as above, and also loop over differrent glob
/// grounding possibilities.
bool PatternMatchEngine::explore_upglob_branches(const PatternTermPtr& ptm,
                                             const Handle& hg,
                                             const Handle& clause_root)
{
	IncomingSet iset;
	if (nullptr == hg->getAtomSpace())
		iset = _pmc.get_incoming_set(hg->getOutgoingAtom(0));
	else
		iset = _pmc.get_incoming_set(hg);

	size_t sz = iset.size();
	DO_LOG({LAZY_LOG_FINE << "Looking globby upward for term = "
	              << ptm->getHandle()->to_string()
	              << "It's grounding " << hg->to_string()
	              << " has " << sz << " branches";})

	// Move up the solution graph, looking for a match.
	bool found = false;
	for (size_t i = 0; i < sz; i++)
	{
		DO_LOG({LAZY_LOG_FINE << "Try upward branch " << i+1 << " of " << sz
		              << " for glob term=" << ptm->to_string()
		              << " propose=" << Handle(iset[i]).value();})

		// Before exploring the link branches, record the current
		// _glob_state size.  The idea is, if the ptm & hg is a match,
		// their state will be recorded in _glob_state, so that one can,
		// if needed, resume and try to ground those globs again in a
		// different way (e.g. backtracking from another branchpoint).
		std::map<GlobPair, GlobState> saved_glob_state;
		saved_glob_state = _glob_state;

		found = explore_glob_branches(ptm, Handle(iset[i]), clause_root);

		// Restore the saved state, for the next go-around.
		_glob_state = saved_glob_state;

		if (found) break;
	}
	DO_LOG({LAZY_LOG_FINE << "Found upward soln = " << found;})
	return found;
}

/// explore_glob_branches -- explore glob grounding alternatives
///
/// Please see the docs for `explore_unordered_branches` for the general
/// idea. In this particular method, all possible alternatives for
/// grounding glob nodes are explored.
bool PatternMatchEngine::explore_glob_branches(const PatternTermPtr& ptm,
                                               const Handle& hg,
                                               const Handle& clause_root)
{
	// Check if the pattern has globs in it,
	OC_ASSERT(0 < _pat->globby_holders.count(ptm->getHandle()),
	          "Glob exploration went horribly wrong!");

	// Record the glob_state *before* starting exploration.
	size_t gstate_size = _glob_state.size();

	// If no solution is found, and there are globs, then there may
	// be other ways to ground the glob differently.  So keep trying,
	// until all possibilities are exhausted.
	//
	// Once there are no more possible ways to ground globby terms,
	// they are removed from glob_state. So simply by comparing the
	// _glob_state size before and after seems to be an OK way to
	// quickly check if we can move on to the next one or not.
	do
	{
		// It's not clear if the odometer can play nice with
		// globby terms. Anyway, no unit test mixs these two.
		// So, for now, we ignore it.
		// if (explore_odometer(ptm, hg, clause_root))
		if (explore_type_branches(ptm, hg, clause_root))
			return true;
		DO_LOG({logger().fine("Globby clause not grounded; try again");})
	}
	while (_glob_state.size() > gstate_size);

	return false;
}

// explore_odometer - explore multiple unordered links at once.
//
// The core issue adressed here is that there may be lots of
// UnorderedLinks below us, and we have to explore all of them.
// So this tries to advance all of them.
// XXX The design here is deeply flawed. Its just barely enough
// to pass the current unit tests, but clearly fails on more
// complex cases. See issue opencog/atomspace#2388
// A redesign is needed.
bool PatternMatchEngine::explore_odometer(const PatternTermPtr& ptm,
                                          const Handle& hg,
                                          const Handle& clause_root)
{
	if (explore_type_branches(ptm, hg, clause_root))
		return true;

	// If no solution was found, and there are unordered links, then
	// there may be alternate permuations of the unordered link that
	// might satisfy this clause. So try those, until exhausted.
	// Note that these unordered links might be buried deeply;
	// that is why we iterate over them here.
	if (_perm_first_term)
	{
		_perm_have_odometer = true;
		DO_LOG({LAZY_LOG_FINE << "First odometer term: "
		                      << _perm_first_term->to_string();})
	}
	if (_perm_latest_term != _perm_first_term)
	{
		DO_LOG({LAZY_LOG_FINE << "Last odometer term: "
		                      << _perm_latest_term->to_string();})
	}

#if 0
// XXX FIXME, this makes SudokuUTest loop forever.
	// If the perm state isn't empty, there must be more!
	if (0 < _perm_state.size())
		_perm_have_more = true;
#endif

	while (nullptr != _perm_have_more)
	{
		_perm_take_step = _perm_have_more;
		_perm_have_more = nullptr;

		DO_LOG({LAZY_LOG_FINE << "Continue exploring term: "
		                      << ptm->to_string();})
		if (explore_type_branches(ptm, hg, clause_root))
		{
			return true;
		}
		if (_perm_latest_wrap and _perm_latest_wrap == _perm_latest_term)
		{
			DO_LOG({LAZY_LOG_FINE << "Terminate Odometer: "
			                      << _perm_latest_term->to_string();})
			return false;
		}
	}
	_perm_have_odometer = false;
	return false;
}

/// explore_unordered_branches -- explore UnorderedLink alternatives.
///
/// Every UnorderedLink of arity N presents N-factorial different
/// grounding possbilities, corresponding to different permutations
/// of the UnorderedLink.  Each permutation must be explored. Thus,
/// this can be thought of as a branching of exploration possibilities,
/// each branch corresponding to a different permutation.  (If you
/// know algebra, then think of the "free object" (e.g. theh "free
/// group") where alternative branches are "free", unconstrained.)
///
/// For each possible branch, the current state is saved, the branch
/// is explored, then the state is popped. If the exploration yielded
/// nothing, then the next branch is explored, until exhaustion of the
/// possibilities.  Upon exhaustion, it returns to the caller.
///
bool PatternMatchEngine::explore_unordered_branches(const PatternTermPtr& ptm,
                                               const Handle& hg,
                                               const Handle& clause_root)
{
	do
	{
		// If the pattern was satisfied, then we are done for good.
		if (explore_single_branch(ptm, hg, clause_root))
			return true;

		DO_LOG({logger().fine("Step to next permutation");})

		// If we are here, there was no match.
		// On the next go-around, take a step.
		_perm_take_step = _perm_have_more;
		_perm_have_more = nullptr;
	}
	while (have_perm(ptm, hg) and _perm_latest_wrap != ptm);

	_perm_take_step = nullptr;
	_perm_have_more = nullptr;
	DO_LOG({logger().fine("No more unordered permutations");})

	return false;
}

/// explore_type_branches -- perform exploration of alternatives.
///
/// This dispatches exploration of different grounding alternatives to
/// one of the "specialist" functions that know how to ground specific
/// link types.
///
/// In the simplest case, there are no alternatives, and this just
/// dispatches to `explore_single_branch()` which is just a wrapper
/// around `tree_compare()`. This just returns true or false to indicate
/// if the suggested grounding `hg` actually is a match for the current
/// term being grounded. Before calling `tree_compare()`, the
/// `explore_single_branch()` method pushes all current state, and then
/// pops it upon return. In other words, this encapsulates a single
/// up-branch (incoming-set branch): grounding of that single branch
/// succeeds or fails. Failure backtracks to the caller of this method;
/// upon return, the current state has been restored; this routine
/// leaves the current state as it found it.
///
/// The `explore_single_branch()` method is part of a recursive chain
/// that only terminates  when a grounding for *the entire pattern* was
/// found (and the grounding was accepted) or if all possibilities were
/// exhaustivelyexplored.  Thus, this returns true only if entire
/// pattern was grounded.
bool PatternMatchEngine::explore_type_branches(const PatternTermPtr& ptm,
                                          const Handle& hg,
                                          const Handle& clause_root)
{
	const Handle& hp = ptm->getHandle();
	Type ptype = hp->get_type();

	// Iterate over different possible choices.
	if (CHOICE_LINK == ptype)
	{
		return explore_choice_branches(ptm, hg, clause_root);
	}

	// Unordered links have permutations to explore.
	if (_nameserver.isA(ptype, UNORDERED_LINK))
	{
		return explore_unordered_branches(ptm, hg, clause_root);
	}

	return explore_single_branch(ptm, hg, clause_root);
}

/// See explore_unordered_branches() for a general explanation.
/// This method handles the ChoiceLink branch alternatives only.
/// This method is never called, currently.
bool PatternMatchEngine::explore_choice_branches(const PatternTermPtr& ptm,
                                                 const Handle& hg,
                                                 const Handle& clause_root)
{
	throw RuntimeException(TRACE_INFO,
		"Maybe this works but its not tested!! Find out!");

	DO_LOG({logger().fine("Begin choice branchpoint iteration loop");})
	do {
		// XXX This `need_choice_push` thing is probably wrong; it probably
		// should resemble the perm_push() used for unordered links.
		// However, currently, no test case trips this up. so .. OK.
		// Whatever. This still probably needs fixing.
		if (_need_choice_push) choice_stack.push(_choice_state);
		bool match = explore_single_branch(ptm, hg, clause_root);
		if (_need_choice_push) POPSTK(choice_stack, _choice_state);
		_need_choice_push = false;

		// If the pattern was satisfied, then we are done for good.
		if (match)
			return true;

		DO_LOG({logger().fine("Step to next choice");})
		// If we are here, there was no match.
		// On the next go-around, take a step.
		_choose_next = true;
	} while (have_choice(ptm, hg));

	DO_LOG({logger().fine("Exhausted all choice possibilities"
	              "\n----------------------------------");})
	return false;
}

/// Check the proposed grounding hg for pattern term hp.
///
/// As the name implies, this will explore only one single potential
/// (proposed) grounding for the current pattern term. This is meant
/// to be called after a viable branch has been identified for
/// exploration.
///
/// This is wrapper around tree compare; if tree_compare
/// returns false, then this returns immediately.
///
/// However, this method is part of the upwards-recursion chain,
/// so if tree_compare approves the proposed grounding, this will
/// recurse upwards, calling do_term_up to get the next pattern
/// term. Thus, this method will return true ONLY if ALL OF the terms
/// and clauses in the pattern are satisfiable (are accepted matches).
///
bool PatternMatchEngine::explore_single_branch(const PatternTermPtr& ptm,
                                               const Handle& hg,
                                               const Handle& clause_root)
{
	solution_push();

	DO_LOG({LAZY_LOG_FINE << "Checking term=" << ptm->to_string()
	              << " for soln by " << hg.value();})

	bool match = tree_compare(ptm, hg, CALL_SOLN);

	if (not match)
	{
		DO_LOG({LAZY_LOG_FINE << "NO solution for term="
		              << ptm->to_string()
		              << " its NOT solved by " << hg.value();})
		solution_pop();
		return false;
	}

	DO_LOG({LAZY_LOG_FINE << "Solved term=" << ptm->getHandle()->to_string()
	              << " it is solved by " << hg->to_string()
	              << ", will move up.";})

	// Continue onwards to the rest of the pattern.
	bool found = do_term_up(ptm, hg, clause_root);

	solution_pop();
	return found;
}

/// do_term_up() -- move upwards from the current term.
///
/// Given the current term, in `hp`, find its parent in the clause,
/// and then call explore_up_branches() to see if the term's parent
/// has corresponding match in the solution graph.
///
/// Note that, in the "normal" case, a given term has only one, unique
/// parent in the given root_clause, and so its easy to find; one just
/// looks at the path from the root clause down to the term, and the
/// parent is the link immediately above it.
///
/// There are five exceptions to this "unique parent" case:
///  * The term is already the root clause; it has no parent. In this
///    case, we send it off to the machinery that explores the next
///    clause.
///  * Exactly the same term may appear twice, 3 times, etc. in the
///    clause, all at different locations.  This is very rare, but
///    can happen. In essence, it has multiple parents; each needs
///    to be checked. We loop over these.
///  * The term is a part of a larger, evaluatable term. In this case,
///    we don't want to go to the immediate parent, we want to go to
///    the larger evaluatable term, and offer that up as the thing to
///    match (i.e. to evaluate, to invoke callbacks, etc.)
///  * The parent is a ChoiceLink. In this case, the ChoiceLink
///    itself cannot be directly matched, as is; only its children can
///    be. So in this case, we fetch the ChoiceLink's parent, instead.
///  * Some crazy combination of the above.
///
/// If it weren't for these complications, this method would be small
/// and simple: it would send the parent to explore_up_branches(), and
/// then explore_up_branches() would respond as to whether it is
/// satisfiable (solvable) or not.
///
/// Takes as an argument an atom `hp` in the pattern, and its matching
/// grounding `hg`.  Thus, hp's parent will need to be matched to hg's
/// parent.
///
/// Returns true if a grounding for the term's parent was found.
///
bool PatternMatchEngine::do_term_up(const PatternTermPtr& ptm,
                                    const Handle& hg,
                                    const Handle& clause_root)
{
	depth = 1;

	// If we are here, then everything below us matches.  If we are
	// at the top of the clause, move on to the next clause. Else,
	// we are working on a term somewhere in the middle of a clause
	// and need to walk upwards.
	const Handle& hp = ptm->getHandle();
	if (term_is_a_clause(ptm, clause_root))
		return clause_accept(clause_root, hg);

	// Move upwards in the term, and hunt for a match, again.
	// There are two ways to move upwards: for a normal term, we just
	// find its parent in the clause. For an evaluatable term, we find
	// the parent evaluatable in the clause, which may be many steps
	// higher.
	DO_LOG({LAZY_LOG_FINE << "Term = " << ptm->to_string()
	              << " " << ptm->getHandle()->to_string()
	              << " of clause = " << clause_root->to_string()
	              << " has ground, move upwards";})

	if (0 < _pat->in_evaluatable.count(hp))
	{
		// If we are here, there are four possibilities:
		// 1) `hp` is not in any evaluatable that lies between it and
		//    the clause root.  In this case, we need to fall through
		//    to the bottom.
		// 2) The evaluatable is the clause root. We evaluate it, and
		//    consider the clause satisfied if the evaluation returns
		//    true. In that case, we continue to the next clause, else we
		//    backtrack.
		// 3) The evaluatable is in the middle of a clause, in which case,
		//    it's parent must be a logical connective: an AndLink, an
		//    OrLink or a NotLink. In this case, we have to loop over
		//    all of the evaluatables within this clause, and connect
		//    them as appropriate. The structure may be non-trivial, so
		//    that presents a challange.  However, it must be logical
		//    connectives all the way up to the root of the clause, so the
		//    easiest thing to do is simply to start at the top, and
		//    recurse downwards.  Ergo, this is much like case 2): the
		//    evaluation either suceeds or fails; we proceed or backtrack.
		// 4) The evaluatable is in the middle of something else. We don't
		//    know what that means, so we throw an error. Actually, this
		//    is too harsh. It may be in the middle of some function that
		//    expects a boolean value as an argument. But I don't know of
		//    any, just right now.
		//
		// Anyway, all of this talk abbout booleans is emphasizing the
		// point that, someday, we need to replace this crisp logic with
		// probabalistic logic of some sort.
		//
		// By the way, if we are here, then `hp` is surely a variable;
		// or, at least, it is, if we are working in the canonical
		// interpretation.

		auto evra = _pat->in_evaluatable.equal_range(hp);
		for (auto evit = evra.first; evit != evra.second; evit++)
		{
			if (not is_unquoted_in_tree(clause_root, evit->second))
				continue;

			logmsg("Term inside evaluatable, move up to it's top:",
			       evit->second);

			// All of the variables occurring in the term should have
			// grounded by now. If not, then its virtual term, and we
			// shouldn't even be here (we can't just backtrack, and
			// try again later).  So validate the grounding, but leave
			// the evaluation for the callback.
// XXX TODO count the number of ungrounded vars !!! (make sure its zero)

			bool found = _pmc.evaluate_sentence(clause_root, var_grounding);
			DO_LOG({logger().fine("After evaluating clause, found = %d", found);})
			if (found)
				return clause_accept(clause_root, hg);

			return false;
		}
	}

	PatternTermPtr parent = ptm->getParent();
	OC_ASSERT(PatternTerm::UNDEFINED != parent, "Unknown term parent");

	const Handle& hi = parent->getHandle();

	// Do the simple case first, ChoiceLinks are harder.
	bool found = false;
	if (CHOICE_LINK != hi->get_type())
	{
		if (explore_up_branches(parent, hg, clause_root)) found = true;
		DO_LOG({logger().fine("After moving up the clause, found = %d", found);})
	}
	else
	if (hi == clause_root)
	{
		DO_LOG({logger().fine("Exploring ChoiceLink at root");})
		if (clause_accept(clause_root, hg)) found = true;
	}
	else
	{
		// If we are here, we have an embedded ChoiceLink, i.e. a
		// ChoiceLink that is not at the clause root. It's contained
		// in some other link, and we have to get that link and
		// perform comparisons on it. i.e. we have to "hop over"
		// (hop up) past the ChoiceLink, before resuming the search.
		// The easiest way to hop is to do it recursively... i.e.
		// call ourselves again.
		DO_LOG({logger().fine("Exploring ChoiceLink below root");})

		OC_ASSERT(not have_choice(parent, hg),
		          "Something is wrong with the ChoiceLink code");

		_need_choice_push = true;
		if (do_term_up(parent, hg, clause_root)) found = true;
	}

	return found;
}

/// This is called when we've navigated to the top of a clause,
/// and so it is fully grounded, and we're essentially done.
/// However, let the callbacks have the final say on whether to
/// proceed onwards, or to backtrack.
///
bool PatternMatchEngine::clause_accept(const Handle& clause_root,
                                       const Handle& hg)
{
	// Is this clause a required clause? If so, then let the callback
	// make the final decision; if callback rejects, then it's the
	// same as a mismatch; try the next one.
	bool match;
	if (is_optional(clause_root))
	{
		clause_accepted = true;
		match = _pmc.optional_clause_match(clause_root, hg, var_grounding);
		DO_LOG({logger().fine("optional clause match callback match=%d", match);})
	}
	else
	if (is_always(clause_root))
	{
		_did_check_forall = true;
		match = _pmc.always_clause_match(clause_root, hg, var_grounding);
		_forall_state = _forall_state and match;
		DO_LOG({logger().fine("for-all clause match callback match=%d", match);})
	}
	else
	{
		match = _pmc.clause_match(clause_root, hg, var_grounding);
		DO_LOG({logger().fine("clause match callback match=%d", match);})
	}
	if (not match) return false;

	if (not is_evaluatable(clause_root))
	{
		clause_grounding[clause_root] = hg;
		logmsg("---------------------\nclause:", clause_root);
		logmsg("ground:", hg);
	}

	// Now go and do more clauses.
	return do_next_clause();
}

/// This is called when all previous clauses have been grounded; so
/// we search for the next one, and try to ground that.
bool PatternMatchEngine::do_next_clause(void)
{
	clause_stacks_push();
	get_next_untried_clause();

	// If there are no further clauses to solve,
	// we are really done! Report the solution via callback.
	if (nullptr == next_clause)
	{
		bool found = report_grounding(var_grounding, clause_grounding);
		DO_LOG(logger().fine("==================== FINITO! accepted=%d", found);)
		DO_LOG(log_solution(var_grounding, clause_grounding);)
		clause_stacks_pop();
		return found;
	}

	Handle joiner = next_joint;
	Handle curr_root = next_clause;

	logmsg("Next clause is", curr_root);
	DO_LOG({LAZY_LOG_FINE << "This clause is "
		              << (is_optional(curr_root)? "optional" : "required");})
	DO_LOG({LAZY_LOG_FINE << "This clause is "
		              << (is_evaluatable(curr_root)?
		                  "dynamically evaluatable" : "non-dynamic");
	logmsg("Joining variable is", joiner);
	logmsg("Joining grounding is", var_grounding[joiner]); })

	// Start solving the next unsolved clause. Note: this is a
	// recursive call, and not a loop. Recursion is halted when
	// the next unsolved clause has no grounding.
	//
	// We continue our search at the variable/glob that "joins"
	// (is shared in common) between the previous (solved) clause,
	// and this clause.

	clause_accepted = false;
	Handle hgnd(var_grounding[joiner]);
	OC_ASSERT(nullptr != hgnd,
	         "Error: joining handle has not been grounded yet!");
	bool found = explore_clause(joiner, hgnd, curr_root);

	// If we are here, and found is false, then we've exhausted all
	// of the search possibilities for the current clause. If this
	// is an optional clause, and no solutions were reported for it,
	// then report the failure of finding a solution now. If this was
	// also the final optional clause, then in fact, we've got a
	// grounding for the whole thing ... report that!
	//
	// Note that lack of a match halts recursion; thus, we can't
	// depend on recursion to find additional unmatched optional
	// clauses; thus we have to explicitly loop over all optional
	// clauses that don't have matches.
	while ((false == found) and
	       (false == clause_accepted) and
	       (is_optional(curr_root)))
	{
		Handle undef(Handle::UNDEFINED);
		bool match = _pmc.optional_clause_match(curr_root, undef, var_grounding);
		DO_LOG({logger().fine("Exhausted search for optional clause, cb=%d", match);})
		if (not match) {
			clause_stacks_pop();
			return false;
		}

		// XXX Maybe should push n pop here? No, maybe not ...
		clause_grounding[curr_root] = Handle::UNDEFINED;
		get_next_untried_clause();
		joiner = next_joint;
		curr_root = next_clause;

		DO_LOG({logmsg("Next optional clause is", curr_root);})
		if (nullptr == curr_root)
		{
			DO_LOG({logger().fine("==================== FINITO BANDITO!");
			log_solution(var_grounding, clause_grounding);})
			found = report_grounding(var_grounding, clause_grounding);
		}
		else
		{
			// Now see if this optional clause has any solutions,
			// or not. If it does, we'll recurse. If it does not,
			// we'll loop around back to here again.
			clause_accepted = false;
			Handle hgnd = var_grounding[joiner];
			found = explore_term_branches(joiner, hgnd, curr_root);
		}
	}

	// If we failed to find anything at this level, we need to
	// backtrack, i.e. pop the stack, and begin a search for
	// other possible matches and groundings.
	clause_stacks_pop();

	return found;
}

/**
 * Search for the next untried, (thus ungrounded, unsolved) clause.
 *
 * The "issued" set contains those clauses which are currently in play,
 * i.e. those for which a grounding is currently being explored. Both
 * grounded, and as-yet-ungrounded clauses may be in this set.  The
 * sole reason of this set is to avoid infinite resursion, i.e. of
 * re-identifying the same clause over and over as unsolved.
 *
 * The words "solved" and "grounded" are used as synonyms through out
 * the code.
 *
 * Additional complications are introduced by the presence of
 * evaluatable terms, black-box terms, and optional clauses. An
 * evaluatable term is any term that needs to be evaluated to determine
 * if it matches: such terms typically do not exist in the atomspace;
 * they are "virtual", and "exist" only when the evaluation returns
 * "true". Thus, these can only be grounded after all other possible
 * clauses are grounded; thus these are saved for last.  It is always
 * possible to save these for last, because earlier stages have
 * guaranteed that all of the non-virtual clauses are connected.
 * Anyway, evaluatables come in two forms: those that can be evaluated
 * quickly, and those that require a "black-box" evaluation of some
 * scheme or python code. Of the two, we save "black-box" for last.
 *
 * Then, after grounding all of the mandatory clauses (virtual or not),
 * we look for optional clauses, if any. Again, these might be virtual,
 * and they might be black...
 *
 * Thus, we use a helper function to broaden the search in each case.
 */
void PatternMatchEngine::get_next_untried_clause(void)
{
	// First, try to ground all the mandatory clauses, only.
	// no virtuals, no black boxes, no optionals.
	if (get_next_thinnest_clause(false, false, false)) return;

	// Don't bother looking for evaluatables if they are not there.
	if (not _pat->evaluatable_holders.empty())
	{
		if (get_next_thinnest_clause(true, false, false)) return;
		if (not _pat->black.empty())
		{
			if (get_next_thinnest_clause(true, true, false)) return;
		}
	}

	// Try again, this time, considering the optional clauses.
	if (not _pat->optionals.empty())
	{
		if (get_next_thinnest_clause(false, false, true)) return;
		if (not _pat->evaluatable_holders.empty())
		{
			if (get_next_thinnest_clause(true, false, true)) return;
			if (not _pat->black.empty())
			{
				if (get_next_thinnest_clause(true, true, true)) return;
			}
		}
	}

	// Now loop over all for-all clauses.
	// I think that all variables will be grounded at this point, right?
	for (const Handle& root : _pat->always)
	{
		if (issued.end() != issued.find(root)) continue;
		issued.insert(root);
		next_clause = root;
		for (const Handle &v : _variables->varset)
		{
			if (is_free_in_tree(root, v))
			{
				next_joint = v;
				return;
			}
		}
		throw RuntimeException(TRACE_INFO, "BUG! Somethings wrong!!");
	}

	// If we are here, there are no more unsolved clauses to consider.
	next_clause = Handle::UNDEFINED;
	next_joint = Handle::UNDEFINED;
}

// Count the number of ungrounded variables in a clause.
//
// This is used to search for the "thinnest" ungrounded clause:
// the one with the fewest ungrounded variables in it. Thus, if
// there is just one variable that needs to be grounded, then this
// can be done in a direct fashion; it resembles the concept of
// "unit propagation" in the DPLL algorithm.
//
// XXX TODO ... Rather than counting the number of variables, we
// should instead look for one with the smallest incoming set.
// That is because the very next thing that we do will be to
// iterate over the incoming set of "pursue" ... so it could be
// a huge pay-off to minimize this.
//
// If there are two ungrounded variables in a clause, then the
// "thickness" is the *product* of the sizes of the two incoming
// sets. Thus, the fewer ungrounded variables, the better.
//
// Danger: this assumes a suitable dataset, as otherwise, the cost
// of this "optimization" can add un-necessarily to the overhead.
//
unsigned int PatternMatchEngine::thickness(const Handle& clause,
                                           const HandleSet& live)
{
	// If there are only zero or one ungrounded vars, then any clause
	// will do. Blow this pop stand.
	if (live.size() < 2) return 1;

	unsigned int count = 0;
	for (const Handle& v : live)
	{
		if (is_unquoted_in_tree(clause, v)) count++;
	}
	return count;
}

/// get_glob_embedding() -- given glob node, return term that it grounds.
///
/// If a GlobNode has a grounding, then there is always some
/// corresponding term which contains that grounded GlobNode and is
/// grounded. If that term appears in two (or more) clauses, then
/// return it, so that it is used as the pivot point to the next
/// ungrounded clause.  If there is no such term, then just  return the
/// glob.
Handle PatternMatchEngine::get_glob_embedding(const Handle& glob)
{
	// If the glob is in only one clause, there is no connectivity map.
	if (0 == _pat->connectivity_map.count(glob)) return glob;

	// Find some clause, any clause at all, containg the glob,
	// that has not been grounded so far. We need to do this because
	// the glob might appear in three clauses, with two of them
	// grounded by a common term, and the third ungrounded
	// with no common term.
	auto clauses =  _pat->connectivity_map.equal_range(glob);
	auto clpr = clauses.first;
	for (; clpr != clauses.second; clpr++)
	{
		if (issued.end() == issued.find(clpr->second)) break;
	}

	// Glob is not in any ungrounded clauses.
	if (clpr == clauses.second) return glob;

	// Typically, the glob appears only once in the clause, so
	// there is only one PatternTerm. The loop really isn't needed.
	HandlePair glbt({glob, clpr->second});
	const auto& ptms = _pat->connected_terms_map.find(glbt);
	for (const PatternTermPtr& ptm : ptms->second)
	{
		// Here, ptm is the glob itself. It will almost surely
		// be in some term. The test for nullptr will surely never
		// trigger.
		const PatternTermPtr& parent = ptm->getParent();
		if (nullptr == parent) return glob;

		// If this term appears in more than one clause, then it
		// can be used as a pivot.
		const Handle& embed = parent->getHandle();
		if ((var_grounding.end() != var_grounding.find(embed)) and
		    (1 < _pat->connectivity_map.count(embed)))
			return embed;
	}
	return glob;
}

/// Same as above, but with three boolean flags:  if not set, then only
/// those clauses satsifying the criterion are considered, else all
/// clauses are considered.
///
/// Return true if we found the next ungrounded clause.
bool PatternMatchEngine::get_next_thinnest_clause(bool search_virtual,
                                                  bool search_black,
                                                  bool search_optionals)
{
	// Search for an as-yet ungrounded clause. Search for required
	// clauses first; then, only if none of those are left, move on
	// to the optional clauses.  We can find ungrounded clauses by
	// looking at the grounded vars, looking up the root, to see if
	// the root is grounded.  If its not, start working on that.
	Handle joint(Handle::UNDEFINED);
	Handle unsolved_clause(Handle::UNDEFINED);
	unsigned int thinnest_joint = UINT_MAX;
	unsigned int thinnest_clause = UINT_MAX;
	bool unsolved = false;

	// Make a list of the as-yet ungrounded variables.
	HandleSet ungrounded_vars;

	// Grounded variables ordered by the size of their grounding incoming set
	std::multimap<std::size_t, Handle> thick_vars;

	for (const Handle &v : _variables->varset)
	{
		const auto& gnd = var_grounding.find(v);
		if (gnd != var_grounding.end())
		{
			// We cannot use GlobNode's directly as joiners, because
			// we don't know how they fit. Instead, we have to fish
			// out a term that contains a grounded glob, and use
			// that term as the joiner.
			if (GLOB_NODE == v->get_type())
			{
				Handle embed = get_glob_embedding(v);
				const Handle& tg = var_grounding[embed];
				std::size_t incoming_set_size = tg->getIncomingSetSize();
				thick_vars.insert(std::make_pair(incoming_set_size, embed));
			}
			else
			{
				std::size_t incoming_set_size = gnd->second->getIncomingSetSize();
				thick_vars.insert(std::make_pair(incoming_set_size, v));
			}
		}
		else ungrounded_vars.insert(v);
	}

	// We are looking for a joining atom, one that is shared in common
	// with the a fully grounded clause, and an as-yet ungrounded clause.
	// The joint is called "pursue", and the unsolved clause that it
	// joins will become our next untried clause. We choose joining atom
	// with smallest size of its incoming set. If there are many such
	// atoms we choose one from clauses with minimal number of ungrounded
	// yet variables.
	for (auto tckvar : thick_vars)
	{
		std::size_t pursue_thickness = tckvar.first;
		const Handle& pursue = tckvar.second;

		if (pursue_thickness > thinnest_joint) break;

		auto root_list = _pat->connectivity_map.equal_range(pursue);

		for (auto it = root_list.first; it != root_list.second; it++)
		{
			const Handle& root = it->second;
			if ((issued.end() == issued.find(root))
			        and (search_virtual or not is_evaluatable(root))
			        and (search_black or not is_black(root))
			        and (search_optionals or not is_optional(root)))
			{
				unsigned int root_thickness = thickness(root, ungrounded_vars);
				if (root_thickness < thinnest_clause)
				{
					thinnest_clause = root_thickness;
					thinnest_joint = pursue_thickness;
					unsolved_clause = root;
					joint = pursue;
					unsolved = true;
				}
			}
		}
	}

	if (unsolved)
	{
		// Joint is a (variable) node that's shared between several
		// clauses. One of the clauses has been grounded, another
		// has not.  We want to now traverse upwards from this node,
		// to find the top of the ungrounded clause.
		next_clause = unsolved_clause;
		next_joint = joint;

		if (unsolved_clause)
		{
			issued.insert(unsolved_clause);
			return true;
		}
	}

	return false;
}

/* ======================================================== */
/**
 * Push all stacks related to the grounding of a clause. This push is
 * meant to be done only when a grounding for a clause has been found,
 * and the next clause is about the be attempted. It saves all of the
 * traversal data associated with the current clause, so that, later
 * on, traversal can be resumed where it was left off.
 *
 * This does NOT push any of the redex stacks because (with the current
 * redex design), all redex substitutions should have terminatated by
 * now, and returned to the main clause. i.e. the redex stack is assumed
 * to be empty, at this point.  (Its possible this design may change in
 * in the future if multi-clause redexes are allowed, whatever the heck
 * that may be!?)
 */
void PatternMatchEngine::clause_stacks_push(void)
{
	_clause_stack_depth++;
	DO_LOG({logger().fine("--- CLAUSE stack push to depth=%d",
	              _clause_stack_depth);})

	var_solutn_stack.push(var_grounding);
	_clause_solutn_stack.push(clause_grounding);

	issued_stack.push(issued);
	choice_stack.push(_choice_state);

	perm_push();

	_pmc.push();
}

/**
 * Pop all clause-traversal-related stacks. This restores state
 * so that the traversal of a single clause can resume where it left
 * off. These do NOT affect any of the redex stacks (which are assumed
 * to be empty at this point.)
 */
void PatternMatchEngine::clause_stacks_pop(void)
{
	_pmc.pop();

	// The grounding stacks are handled differently.
	POPSTK(_clause_solutn_stack, clause_grounding);
	POPSTK(var_solutn_stack, var_grounding);
	POPSTK(issued_stack, issued);

	POPSTK(choice_stack, _choice_state);

	perm_pop();

	_clause_stack_depth --;
	DO_LOG({logger().fine("CLAUSE stack pop to depth %d", _clause_stack_depth);})
}

/**
 * Unconditionally clear all graph traversal stacks
 * XXX TODO -- if the algo is working correctly, then all
 * of these should already be empty, when this method is
 * called. So really, we should check the stack size, and
 * assert if it is not zero ...
 */
void PatternMatchEngine::clause_stacks_clear(void)
{
	_clause_stack_depth = 0;
#if 0
	// Currently, only GlobUTest fails when this is uncommented.
	OC_ASSERT(0 == _clause_solutn_stack.size());
	OC_ASSERT(0 == var_solutn_stack.size());
	OC_ASSERT(0 == issued_stack.size());
	OC_ASSERT(0 == choice_stack.size());
	OC_ASSERT(0 == perm_stack.size());
#else
	while (!_clause_solutn_stack.empty()) _clause_solutn_stack.pop();
	while (!var_solutn_stack.empty()) var_solutn_stack.pop();
	while (!issued_stack.empty()) issued_stack.pop();
	while (!choice_stack.empty()) choice_stack.pop();
	while (!perm_stack.empty()) perm_stack.pop();
#endif
}

void PatternMatchEngine::solution_push(void)
{
	var_solutn_stack.push(var_grounding);
	_clause_solutn_stack.push(clause_grounding);
}

void PatternMatchEngine::solution_pop(void)
{
	POPSTK(var_solutn_stack, var_grounding);
	POPSTK(_clause_solutn_stack, clause_grounding);
}

void PatternMatchEngine::solution_drop(void)
{
	var_solutn_stack.pop();
	_clause_solutn_stack.pop();
}

/* ======================================================== */

/// Pass the grounding that was found out to the callback.
/// ... unless there is an Always clasue, in which case we
/// save them up until we've looked at all of them.
bool PatternMatchEngine::report_grounding(const HandleMap &var_soln,
                                          const HandleMap &term_soln)
{
	// If there is no for-all clause (no AlwaysLink clause)
	// then report groundings as they are found.
	if (_pat->always.size() == 0)
		return _pmc.grounding(var_soln, term_soln);

	// If we are here, we need to record groundings, until later,
	// when we find out if the for-all clauses were satsified.

	// Don't even bother caching, if we know we are losing.
	if (not _forall_state) return false;

	_var_ground_cache.push_back(var_soln);
	_term_ground_cache.push_back(term_soln);

	// Keep going.
	return false;
}

bool PatternMatchEngine::report_forall(void)
{
	// Nothing to do.
	if (_pat->always.size() == 0) return false;

	// If its OK to report, then report them now.
	bool halt = false;
	if (_forall_state)
	{
		size_t nitems = _var_ground_cache.size();
		OC_ASSERT(_term_ground_cache.size() == nitems);
		for (size_t i=0; i<nitems; i++)
		{
			halt = _pmc.grounding(_var_ground_cache[i],
			                      _term_ground_cache[i]);
			if (halt) break;
		}
	}
	_forall_state = true;
	_var_ground_cache.clear();
	_term_ground_cache.clear();
	return halt;
}

/* ======================================================== */

/**
 * explore_neighborhood - explore the local (connected) neighborhood
 * of the starter clause, looking for a match.  The idea here is that
 * it is much easier to traverse a connected graph looking for the
 * appropriate subgraph (pattern) than it is to try to explore the
 * whole atomspace, at random.  The user callback `initiate_search()`
 * should call this method, suggesting a clause to start with, and
 * where in the clause the search should begin.
 *
 * Inputs:
 * do_clause: must be one of the clauses previously specified in the
 *            clause list of the match() method.
 * term:      must be a sub-clause of do_clause; that is, must be a link
 *            that appears in do_clause. Must contain `grnd` below.
 * grnd:      must be a (non-variable) node in the `term` term.
 *            That is, this must be one of the outgoing atoms of the
 *            `term` link; it must be a node, and it must not be
 *            a variable node or glob node.
 *
 * Returns true if one (or more) matches are found
 *
 * This routine is meant to be invoked on every candidate atom taken
 * from the atom space. That atom is assumed to anchor some part of
 * a graph that hopefully will match the pattern.
 */
bool PatternMatchEngine::explore_neighborhood(const Handle& do_clause,
                                              const Handle& term,
                                              const Handle& grnd)
{
	clause_stacks_clear();
	bool halt = explore_redex(term, grnd, do_clause);
	bool stop = report_forall();
	return halt or stop;
}

/**
 * Same as above, obviously; we just pick up the graph context
 * where we last left it.
 */
bool PatternMatchEngine::explore_redex(const Handle& term,
                                       const Handle& grnd,
                                       const Handle& first_clause)
{
	// Cleanup
	clear_current_state();

	// Match the required clauses.
	issued.insert(first_clause);
	return explore_clause(term, grnd, first_clause);
}

/**
 * Every clause in a pattern is one of two types:  it either
 * specifies a pattern to be matched, or it specifies an evaluatable
 * atom that must be evaluated to determine if it is to be accepted.
 * (In the default callback, evaluatable atoms are always crisp
 * boolean-logic formulas, although the infrastructure is designed
 * to handle other situations as well, e.g. Bayesian formulas, etc.)
 *
 * This method simply dispatches a given clause to be either pattern
 * matched, or to be evaluated.
 */
bool PatternMatchEngine::explore_clause(const Handle& term,
                                        const Handle& grnd,
                                        const Handle& clause)
{
	// If we are looking for a pattern to match, then ... look for it.
	// Evaluatable clauses are not patterns; they are clauses that
	// evaluate to true or false.
	if (not is_evaluatable(clause))
	{
		DO_LOG({logger().fine("Clause is matchable; start matching it");})

		_did_check_forall = false;
		bool found = explore_term_branches(term, grnd, clause);

		if (not _did_check_forall and is_always(clause))
		{
			// We need to record failures for the AlwaysLink
			Handle empty;
			_forall_state = _forall_state and
				_pmc.always_clause_match(clause, empty, var_grounding);
		}

		// If found is false, then there's no solution here.
		// Bail out, return false to try again with the next candidate.
		return found;
	}

	// If we are here, we have an evaluatable clause on our hands.
	DO_LOG({logger().fine("Clause is evaluatable; start evaluating it");})

	// Some people like to have a clause that is just one big
	// giant variable, matching almost anything. Keep these folks
	// happy, and record the suggested grounding. There's nowhere
	// else to do this, so we do it here.
	if (term->get_type() == VARIABLE_NODE)
		var_grounding[term] = grnd;

	bool found = _pmc.evaluate_sentence(clause, var_grounding);
	DO_LOG({logger().fine("Post evaluating clause, found = %d", found);})
	if (found)
	{
		return clause_accept(clause, grnd);
	}
	else if (is_always(clause))
	{
		// We need to record failures for the AlwaysLink
		Handle empty;
		_forall_state = _forall_state and
			_pmc.always_clause_match(clause, empty, var_grounding);
	}

	return false;
}

void PatternMatchEngine::record_grounding(const PatternTermPtr& ptm,
                                          const Handle& hg)
{
	const Handle& hp = ptm->getHandle();

	// If this is a closed pattern, not containing any variables,
	// then there is no need to save it.
	if (hp == hg)
		return;

	// Only record if the pattern is not quoted, otherwise the pattern
	// is not completely self-contained.
	if (not ptm->isQuoted())
		var_grounding[hp] = hg;
	// If it is quoted, maybe the quote is in the ptm. Unwrap it,
	// and record that. (Huh ???)
	else if (const Handle& quote = ptm->getQuote())
		var_grounding[quote] = hg;
}

/**
 * Clear current traversal state. This gets us into a state where we
 * can start traversing a set of clauses.
 */
void PatternMatchEngine::clear_current_state(void)
{
	// Clear all state.
	var_grounding.clear();
	clause_grounding.clear();

	depth = 0;

	// ChoiceLink state
	_choice_state.clear();
	_need_choice_push = false;
	_choose_next = true;

	// UnorderedLink state
	_perm_have_more = nullptr;
	_perm_take_step = nullptr;
	_perm_reset = false;
	_perm_have_odometer = false;
	_perm_first_term = nullptr;
	_perm_latest_term = nullptr;
	_perm_latest_wrap = nullptr;
	_perm_state.clear();

	// GlobNode state
	_glob_state.clear();

	issued.clear();
}

bool PatternMatchEngine::explore_constant_evaluatables(const HandleSeq& clauses)
{
	bool found = true;
	for (const Handle& clause : clauses) {
		if (is_in(clause, _pat->evaluatable_holders)) {
			found = _pmc.evaluate_sentence(clause, HandleMap());
			if (not found)
				break;
		}
	}
	if (found)
		report_grounding(HandleMap(), HandleMap());

	return found;
}

PatternMatchEngine::PatternMatchEngine(PatternMatchCallback& pmcb)
	: _pmc(pmcb),
	_nameserver(nameserver()),
	_variables(nullptr),
	_pat(nullptr),
	clause_accepted(false)
{
	// current state
	depth = 0;

	// graph state
	_clause_stack_depth = 0;

	// choice link state
	_need_choice_push = false;
	_choose_next = true;

	// unordered link state
	_perm_have_more = nullptr;
	_perm_take_step = nullptr;
	_perm_reset = false;
	_perm_have_odometer = false;
	_perm_first_term = nullptr;
	_perm_latest_term = nullptr;
	_perm_latest_wrap = nullptr;
}

void PatternMatchEngine::set_pattern(const Variables& v,
                                     const Pattern& p)
{
	_variables = &v;
	_pat = &p;
}

/* ======================================================== */

#ifdef DEBUG
void PatternMatchEngine::print_solution(
	const HandleMap &vars,
	const HandleMap &clauses)
{
	Logger::Level save = logger().get_level();
	logger().set_level("fine");
	logger().set_timestamp_flag(false);
	logger().set_print_to_stdout_flag(true);
	logger().set_print_level_flag(false);
	log_solution(vars, clauses);

	logger().set_level(save);
}

void PatternMatchEngine::log_solution(
	const HandleMap &vars,
	const HandleMap &clauses)
{
	if (!logger().is_fine_enabled())
		return;

	logger().fine() << "There are groundings for " << vars.size() << " terms";
	int varcnt = 0;
	for (const auto& j: vars)
	{
		Type vtype = j.first->get_type();
		if (VARIABLE_NODE == vtype or GLOB_NODE == vtype) varcnt++;
	}
	logger().fine() << "Groundings for " << varcnt << " variables:";

	// Print out the bindings of solutions to variables.
	for (const auto& j: vars)
	{
		Handle var(j.first);
		Handle soln(j.second);

		// Only print grounding for variables.
		Type vtype = var->get_type();
		if (VARIABLE_NODE != vtype and GLOB_NODE != vtype) continue;

		if (not soln)
		{
			logger().fine("ERROR: ungrounded variable %s\n",
			              var->to_short_string().c_str());
			continue;
		}

		logger().fine("\t%s maps to %s\n",
		              var->to_short_string().c_str(),
		              soln->to_short_string().c_str());
	}

	// Print out the full binding to all of the clauses.
	logger().fine() << "Groundings for " << clauses.size() << " clauses:";
	HandleMap::const_iterator m;
	int i = 0;
	for (m = clauses.begin(); m != clauses.end(); ++m, ++i)
	{
		if (not m->second)
		{
			Handle mf(m->first);
			logmsg("ERROR: ungrounded clause", mf);
			continue;
		}
		std::string str = m->second->to_short_string();
		logger().fine("%d.   %s", i, str.c_str());
	}
}

/**
 * For debug logging only
 */
void PatternMatchEngine::log_term(const HandleSet &vars,
                                  const HandleSeq &clauses)
{
	logger().fine("Clauses:");
	for (Handle h : clauses) log(h);

	logger().fine("Vars:");
	for (Handle h : vars) log(h);
}
#else

void PatternMatchEngine::log_solution(const HandleMap &vars,
                                      const HandleMap &clauses) {}

void PatternMatchEngine::log_term(const HandleSet &vars,
                                  const HandleSeq &clauses) {}
#endif

/* ===================== END OF FILE ===================== */
