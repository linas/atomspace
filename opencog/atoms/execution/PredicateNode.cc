/*
 * opencog/atoms/execution/PredicateNode.cc
 *
 * Copyright (C) 2019 Linas Vepstas
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "PredicateNode.h"

using namespace opencog;

PredicateNode::PredicateNode(Type t, const std::string& s)
	: Node(t, s)
{
	if (not nameserver().isA(t, PREDICATE_NODE))
		throw RuntimeException(TRACE_INFO,
			"Expecting an PredicateNode or an inherited type thereof");
}

PredicateNode::PredicateNode(const Node& n)
	: Node(n)
{
	Type tscope = n.get_type();
	if (not nameserver().isA(tscope, PREDICATE_NODE))
		throw RuntimeException(TRACE_INFO,
			"Expecting an PredicateNode or an inherited type thereof");
}

DEFINE_NODE_FACTORY(PredicateNode, PREDICATE_NODE)
