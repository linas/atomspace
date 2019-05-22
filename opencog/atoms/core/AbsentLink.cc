/*
 * AbsentLink.cc
 *
 * Copyright (C) 2015, 2019 Linas Vepstas
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

	throw InvalidParamException(TRACE_INFO,
		"Fully grounded AbsentLinks cannot be placed in an AtomSpace! Got %s",
		to_string().c_str());
}

AbsentLink::AbsentLink(const HandleSeq& oset, Type type)
	: FreeLink(oset, type)
{
	init();
}

AbsentLink::AbsentLink(const Link &l)
	: FreeLink(l)
{
	// Type must be as expected
	Type tscope = l.get_type();
	if (not nameserver().isA(tscope, ABSENT_LINK))
	{
		const std::string& tname = nameserver().getTypeName(tscope);
		throw InvalidParamException(TRACE_INFO,
			"Expecting a AbsentLink, got %s", tname.c_str());
	}

	init();
}

TruthValuePtr AbsentLink::evaluate(AtomSpace* as, bool uh)
{
printf("duuuuuuude Absent link evaluate!!!\n");
	return TruthValue::TRUE_TV();
}

DEFINE_LINK_FACTORY(AbsentLink, ABSENT_LINK)

/* ===================== END OF FILE ===================== */
