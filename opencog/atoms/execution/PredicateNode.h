/*
 * opencog/atoms/execution/PredicateNode.h
 *
 * Copyright (C) 2019 Linas Vepstas
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef _OPENCOG_PREDICATE_NODE_H
#define _OPENCOG_PREDICATE_NODE_H

#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/execution/EvaluationLink.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

class AtomSpace;

class PredicateNode : public Node
{
public:
	PredicateNode(Type, const std::string&);
	PredicateNode(const Node&);

	bool is_executable() { return true; }
	ValuePtr execute(AtomSpace* as, bool silent) {
		return
			CastToValue(EvaluationLink::do_evaluate(as, this->get_handle(), silent));
	}

	static Handle factory(const Handle&);
};

typedef std::shared_ptr<PredicateNode> PredicateNodePtr;
static inline PredicateNodePtr PredicateNodeCast(const Handle& h)
   { AtomPtr a(h); return std::dynamic_pointer_cast<PredicateNode>(a); }
static inline PredicateNodePtr PredicateNodeCast(AtomPtr a)
   { return std::dynamic_pointer_cast<PredicateNode>(a); }

#define createPredicateNode std::make_shared<PredicateNode>

/** @}*/
}

#endif // _OPENCOG_PREDICATE_NODE_H
