/*
 * opencog/atoms/reduct/GreaterThanLink.cc
 *
 * Copyright (C) 2015, 2018 Linas Vepstas
 * All Rights Reserved
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <opencog/atoms/atom_types/atom_types.h>
#include <opencog/atoms/base/ClassServer.h>
#include <opencog/atoms/core/NumberNode.h>
#include "ArithmeticLink.h"
#include "GreaterThanLink.h"
#include "MinusLink.h"

using namespace opencog;

GreaterThanLink::GreaterThanLink(const HandleSeq&& oset, Type t)
    : FunctionLink(std::move(oset), t)
{
	init();
}

GreaterThanLink::GreaterThanLink(const Handle& a)
    : FunctionLink({a}, GREATER_THAN_LINK)
{
	init();
}

GreaterThanLink::GreaterThanLink(const Handle& a, const Handle& b)
    : FunctionLink({a, b}, GREATER_THAN_LINK)
{
	init();
}

void GreaterThanLink::init(void)
{
	Type tscope = get_type();
	if (not nameserver().isA(tscope, GREATER_THAN_LINK))
		throw InvalidParamException(TRACE_INFO, "Expecting a GreaterThanLink");

	size_t nargs = _outgoing.size();
	if (0 == nargs or 2 < nargs)
		throw InvalidParamException(TRACE_INFO,
			"GreaterThanLink expects one or two arguments, got %s",
			to_string().c_str());
}

// ============================================================

ValuePtr GreaterThanLink::execute(AtomSpace* as, bool silent)
{
	ValuePtr vi;
	if (2 == _outgoing.size())
	{
		MinusLinkPtr diff = createMinusLink(_outgoing[0], _outgoing[1]);
		vi = diff->execute();
	}
	else
	{
		vi = ArithmeticLink::get_value(as, silent, _outgoing[0]);
	}

	Type vitype = vi->get_type();

	if (NUMBER_NODE == vitype)
	{
		const std::vector<double>& dvec(NumberNodeCast(vi)->value());
		std::vector<double> gtvec;
		for (double dv : dvec)
		{
			if (dv > 0.0) gtvec.push_back(1.0);
			else gtvec.push_back(0.0);
		}
		return createNumberNode(gtvec);
	}

	if (nameserver().isA(vitype, FLOAT_VALUE))
	{
		const std::vector<double>& dvec(FloatValueCast(vi)->value());
		std::vector<double> gtvec;
		for (double dv : dvec)
		{
			if (dv > 0.0) gtvec.push_back(1.0);
			else gtvec.push_back(0.0);
		}
		return createFloatValue(gtvec);
	}

	return createNumberNode(0.0);
}

DEFINE_LINK_FACTORY(GreaterThanLink, GREATER_THAN_LINK);

// ============================================================
