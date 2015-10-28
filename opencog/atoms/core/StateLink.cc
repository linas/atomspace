/*
 * StateLink.cc
 *
 * Copyright (C) 2015 Linas Vepstas
 *
 * Author: Linas Vepstas <linasvepstas@gmail.com>  May 2015
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the
 * exceptions at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <opencog/atomspace/ClassServer.h>

#include "StateLink.h"

using namespace opencog;

void StateLink::init()
{
	// Must have name and body
	size_t sz = _outgoing.size();
	if (2 != sz and 1 != sz)
		throw InvalidParamException(TRACE_INFO,
			"Expecting name and state, got size %d", _outgoing.size());

	FreeLink::init();
}

StateLink::StateLink(const HandleSeq& oset,
                       TruthValuePtr tv, AttentionValuePtr av)
	: UniqueLink(STATE_LINK, oset, tv, av)
{
	init();
}

StateLink::StateLink(const Handle& name, const Handle& defn,
                       TruthValuePtr tv, AttentionValuePtr av)
	: UniqueLink(STATE_LINK, HandleSeq({name, defn}), tv, av)
{
	init();
}

StateLink::StateLink(Link &l)
	: UniqueLink(l)
{
	init();
}

/**
 * Get the state associated with the alias.
 * This will be the second atom of some StateLink, where
 * `alias` is the first.
 */
Handle StateLink::get_state(const Handle& alias)
{
	Handle uniq(get_unique(alias, STATE_LINK, true));
	LinkPtr luniq(LinkCast(uniq));
	return luniq->getOutgoingAtom(1);
}

/**
 * Get the link associated with the alias.  This will be the StateLink
 * which has `alias` as the first member of the outgoing set.
 */
Handle StateLink::get_link(const Handle& alias)
{
	return get_unique(alias, STATE_LINK, true);
}

/**
 * If there is a *second* StateLink, equivalent to this one,
 * return it.  This is used for managing state in the AtomSpace.
 */
Handle StateLink::get_other(void) const
{
	// No-op if this has variables!
	if (0 < this->_vars.varseq.size()) return Handle();

	// Get all StateLinks associated with the alias. Ignore this one.
	const Handle& alias = _outgoing[0];
	IncomingSet defs = alias->getIncomingSetByType(STATE_LINK);

	// Return any non-unique definition that isn't this one,
	// and doesn't have variables in it.  Multiple "open terms"
	// are OK, and naturally occur in patterns.
	for (const LinkPtr& defl : defs)
	{
		if (defl->getOutgoingAtom(0) == alias and defl.get() != this)
		{
			FreeLinkPtr flp(FreeLinkCast(defl));
			if (0 < flp->get_vars().varseq.size()) continue;
			return defl->getHandle();
		}
	}
	return Handle();
}

Handle StateLink::execute(AtomSpace* as) const
{
#if THIS_FAILS_SEE_BELOW
	return get_state(_outgoing[0]);
#endif

// #define THIS_CREATES_CONFUSION
#ifdef THIS_CREATES_CONFUSION
	// Chances are good that `this` has not yet been inserted in any
	// atomspace, because it has been freshly minted with PutLink.
	// This will be our only chance to get it into an atomspace,
	// and so we do that.  Note: the get_state() call will fail if
	// `this` is not in some atomspace.
	Handle self(as->add_atom(((Atom*) this)->shared_from_this()));
	LinkPtr lself(LinkCast(self));
	return get_state(lself->getOutgoingAtom(0));
#endif

#define THIS_SHOULD_WORK 1
#ifdef THIS_SHOULD_WORK
	if (_outgoing.size() != 1)
		return ((Atom*) this)->getHandle();

	return get_state(_outgoing[0]);
#endif
}

/* ===================== END OF FILE ===================== */
