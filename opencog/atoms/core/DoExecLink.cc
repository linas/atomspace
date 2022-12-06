/*
 * DoExecLink.cc
 *
 * Copyright (C) 2017, 2022 Linas Vepstas
 *
 * Author: Linas Vepstas <linasvepstas@gmail.com>  January 2009, 2015, 2017
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
#include "DoExecLink.h"

using namespace opencog;

DoExecLink::DoExecLink(const HandleSeq&& oset, Type t)
	: Link(std::move(oset), t)
{
	if (not nameserver().isA(t, DO_EXEC_LINK))
	{
		const std::string& tname = nameserver().getTypeName(t);
		throw InvalidParamException(TRACE_INFO,
			"Expecting an DoExecLink, got %s", tname.c_str());
	}
}

// ---------------------------------------------------------------

/// Execute everything, if possible.
ValuePtr DoExecLink::execute(AtomSpace*as, bool silent)
{
	// In principle, we should be calling Instantiator to
	// perform the execution, so that e.g. PutLink gets
	// handled correctly. In practice, we're going to punt:
	// the correct long-term solution is to fix PutLink,
	// instead of adding hacks here.
	HandleSeq oset;
	for (const Handle& h : _outgoing)
	{
		if (h->is_type(EXECUTABLE_LINK))
			oset.emplace_back(HandleCast(h->execute()));
		else
			oset.push_back(h);
	}

	return as->add_link(_type, std::move(oset));
}

DEFINE_LINK_FACTORY(DoExecLink, DO_EXEC_LINK)

/* ===================== END OF FILE ===================== */
