/*
 * opencog/atoms/execution/EvaluatableLink.cc
 *
 * Copyright (C) 2019 Linas Vepstas
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "EvaluatableLink.h"

using namespace opencog;

EvaluatableLink::EvaluatableLink(const HandleSeq& oset, Type t)
    : Link(oset, t)
{
	if (not nameserver().isA(t, EVALUATABLE_LINK))
		throw RuntimeException(TRACE_INFO,
		    "Expecting an EvaluatableLink or an inherited type thereof");
}

EvaluatableLink::EvaluatableLink(const Link& l)
    : Link(l)
{
	Type tscope = l.get_type();
	if (EVALUATABLE_LINK != tscope)
		throw RuntimeException(TRACE_INFO,
		    "Expecting an EvaluatableLink");
}

DEFINE_LINK_FACTORY(EvaluatableLink, EVALUATABLE_LINK)
