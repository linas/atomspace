/*
 * opencog/atoms/reduct/BoolLink.cc
 *
 * Copyright (C) 2015,2018,2022 Linas Vepstas
 * All Rights Reserved
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <opencog/atoms/atom_types/atom_types.h>
#include <opencog/atoms/base/ClassServer.h>
#include <opencog/atoms/value/BoolValue.h>
#include "BoolLink.h"

using namespace opencog;

BoolLink::BoolLink(const HandleSeq&& oset, Type t)
    : Link(std::move(oset), t)
{
	init();
}

void BoolLink::init(void)
{
	Type tscope = get_type();
	if (not nameserver().isA(tscope, BOOL_LINK))
		throw InvalidParamException(TRACE_INFO, "Expecting a BoolLink");
}

// ============================================================

ValuePtr BoolLink::execute(AtomSpace* as, bool silent)
{
	size_t sz = size();
	if (0 == sz)
	{
		if (SEQUENTIAL_OR_LINK == get_type()) return createBoolValue(false);
		if (SEQUENTIAL_AND_LINK == get_type()) return createBoolValue(true);
		throw InvalidParamException(TRACE_INFO, "NotLink expects an argument");
	}

	// Unfortunately, there are many places where this link is used
	// in a casual manner, purely as a declarative, and yet people
	// wish to execute it in those contexts. This is arguably just
	// an outright fail. But, to keep the peace, we'll allow this
	// usage for now.
	if (not _outgoing[0]->is_executable())
		return shared_from_this();

	ValuePtr vp = _outgoing[0]->execute(as, silent);
	if (1 == sz and NOT_LINK != get_type()) return vp;

	if (not nameserver().isA(vp->get_type(), BOOL_VALUE)) return vp;

printf("duuude after execution its %s\n", vp->to_string().c_str());
	BoolValuePtr bvp = BoolValueCast(vp);
	std::vector<bool> bv = bvp->value();

	if (NOT_LINK == get_type())
	{
		if (1 != sz)
			throw InvalidParamException(TRACE_INFO, "NotLink expects one argument");
		bv = bool_not(bv);
		return createBoolValue(bv);
	}

	if (SEQUENTIAL_OR_LINK == get_type())
	{
		for (size_t i=1; i<sz; i++)
		{
			BoolValuePtr av = BoolValueCast(_outgoing[i]->execute(as, silent));
			bv = bool_or(av->value(), bv);
		}
		return createBoolValue(bv);
	}

	for (size_t i=1; i<sz; i++)
	{
		BoolValuePtr av = BoolValueCast(_outgoing[i]->execute(as, silent));
		bv = bool_and(av->value(), bv);
	}
	return createBoolValue(bv);
}

DEFINE_LINK_FACTORY(BoolLink, BOOL_LINK);

// ============================================================
