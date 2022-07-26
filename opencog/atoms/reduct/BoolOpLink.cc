/*
 * opencog/atoms/reduct/BoolOpLink.cc
 *
 * Copyright (C) 2015,2018,2022 Linas Vepstas
 * All Rights Reserved
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <opencog/atoms/atom_types/atom_types.h>
#include <opencog/atoms/base/ClassServer.h>
#include <opencog/atoms/value/BoolValue.h>
#include "BoolOpLink.h"

using namespace opencog;

BoolOpLink::BoolOpLink(const HandleSeq&& oset, Type t)
    : UnorderedLink(std::move(oset), t)
{
	init();
}

void BoolOpLink::init(void)
{
	Type tscope = get_type();
	if (not nameserver().isA(tscope, BOOL_OP_LINK))
		throw InvalidParamException(TRACE_INFO, "Expecting a BoolOpLink");
}

// ============================================================

ValuePtr BoolOpLink::execute(AtomSpace* as, bool silent)
{
	size_t sz = size();
	if (0 == sz)
	{
		if (OR_LINK == get_type()) return createBoolValue(false);
		createBoolValue(true);
	}

	ValuePtr vp = _outgoing[0]->execute(as, silent);
	if (1 == sz) return vp;

	BoolValuePtr bvp = BoolValueCast(vp);
	std::vector<bool> bv = bvp->value();

	if (OR_LINK == get_type())
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

DEFINE_LINK_FACTORY(BoolOpLink, BOOL_OP_LINK);

// ============================================================
