/*
 * opencog/atoms/reduct/SineLink.cc
 *
 * Copyright (C) 2020,2022 Linas Vepstas
 * All Rights Reserved
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <opencog/atoms/atom_types/atom_types.h>
#include <opencog/atoms/base/ClassServer.h>
#include <opencog/atoms/core/NumberNode.h>
#include "SineLink.h"

using namespace opencog;

SineLink::SineLink(const HandleSeq&& oset, Type t)
    : NumericFunctionLink(std::move(oset), t)
{
	init();
}

SineLink::SineLink(const Handle& a)
    : NumericFunctionLink({a}, SINE_LINK)
{
	init();
}

void SineLink::init(void)
{
	Type tscope = get_type();
	if (not nameserver().isA(tscope, SINE_LINK))
		throw InvalidParamException(TRACE_INFO, "Expecting a SineLink");

	size_t nargs = _outgoing.size();
	if (1 != nargs)
		throw InvalidParamException(TRACE_INFO,
			"SineLink expects one, got %s",
			to_string().c_str());
}

// ============================================================

ValuePtr SineLink::execute(AtomSpace* as, bool silent)
{
	ValuePtr reduction;
	ValuePtr result(apply_func(as, silent,
		_outgoing[0], sin, reduction));

	if (result) return result;

	// No numeric values available. Sorry!
	// Return the best-possible reduction that we did get.
	if (reduction->is_atom())
		return createSineLink(HandleCast(reduction));

	// Unable to reduce at all. Just return the original atom.
	return get_handle();
}

DEFINE_LINK_FACTORY(SineLink, SINE_LINK);

// ============================================================
