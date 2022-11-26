/*
 * FetchValueOfLink.cc
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

#include <opencog/persist/api/StorageNode.h>

#include "FetchValueOfLink.h"

using namespace opencog;

FetchValueOfLink::FetchValueOfLink(const HandleSeq&& oset, Type t)
	: ValueOfLink(std::move(oset), t)
{
	if (not nameserver().isA(t, FETCH_VALUE_OF_LINK))
	{
		const std::string& tname = nameserver().getTypeName(t);
		throw InvalidParamException(TRACE_INFO,
			"Expecting an FetchValueOfLink, got %s", tname.c_str());
	}
	init();
}

void FetchValueOfLink::init(void)
{
	size_t ary = _outgoing.size();

	if (3 != ary and 4 != ary)
	throw SyntaxException(TRACE_INFO, "Expecting three or four atoms!");
}

// ---------------------------------------------------------------

/// Fetch the Value first, and then return it.
ValuePtr FetchValueOfLink::execute(AtomSpace* as, bool silent)
{
	StorageNodePtr stnp = StorageNodeCast(_outgoing[2]);

	// If the StorageNode is not open for reading, it
	// will either throw, or do something else. Not our decision.
	stnp->fetch_value(_outgoing[0], _outgoing[1], as);

	if (3 == _outgoing.size())
		return ValueOfLink::do_execute(as, silent, -1);
	return ValueOfLink::do_execute(as, silent, 3);
}

DEFINE_LINK_FACTORY(FetchValueOfLink, FETCH_VALUE_OF_LINK)

/* ===================== END OF FILE ===================== */
