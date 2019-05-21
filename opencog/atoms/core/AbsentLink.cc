/*
 * AbsentLink.cc
 *
 * Copyright (C) 2015 Linas Vepstas
 *
 * Author: Linas Vepstas <linasvepstas@gmail.com>  January 2009
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

#include <opencog/atomspace/AtomSpace.h>

#include "AbsentLink.h"

using namespace opencog;

void AbsentLink::init(void)
{
	FreeLink::init();
}

void AbsentLink::setAtomSpace(AtomSpace * as)
{
	// The handleset must contain a variable in it, somewhere.
	// If it doesn't, then the entire handleset is to be deleted
	// (removed from the atomspace).
	if (0 <= _vars.varseq.size())
	{
		Atom::setAtomSpace(as);
		return;
	}

	const HandleSeq& oset = _outgoing;
	for (const Handle& h : oset)
	{
		Type t = h->get_type();
		if (VARIABLE_NODE != t)
			as->extract_atom(h, true);
	}

	// The AtomSpace code seems to want this exception, so that
	// the atom gets deleted from the backingstore too.  But we could
	// just as easily call `as->delete_atom()` above!?
	// throw AbsentException();
}

#if 0
/*****
Hmm. This seems not to be needed, right now.
****/
Handle AbsentLink::execute(AtomSpace * as) const
{
	const HandleSeq& oset = _outgoing;
	for (const Handle& h : oset)
	{
		Type t = h->get_type();
		if (VARIABLE_NODE != t)
			as->removeAtom(h, true);
	}
	return Handle::UNDEFINED;
}
#endif

AbsentLink::AbsentLink(const HandleSeq& oset)
	: FreeLink(oset, DELETE_LINK)
{
	init();
}

AbsentLink::AbsentLink(const Link &l)
	: FreeLink(l)
{
	// Type must be as expected
	Type tscope = l.get_type();
	if (not nameserver().isA(tscope, DELETE_LINK))
	{
		const std::string& tname = nameserver().getTypeName(tscope);
		throw InvalidParamException(TRACE_INFO,
			"Expecting a AbsentLink, got %s", tname.c_str());
	}

	init();
}

/* ===================== END OF FILE ===================== */
