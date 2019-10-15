/*
 * opencog/atoms/execution/EvaluatableLink.h
 *
 * Copyright (C) 2019 Linas Vepstas
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef _OPENCOG_EVALUATABLE_LINK_H
#define _OPENCOG_EVALUATABLE_LINK_H

#include <opencog/atoms/base/Link.h>
#include <opencog/atoms/execution/EvaluationLink.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

class AtomSpace;

class EvaluatableLink : public Link
{
public:
	EvaluatableLink(const HandleSeq&, Type=EVALUATABLE_LINK);
	EvaluatableLink(const Link& l);

	bool is_executable() { return true; }
	ValuePtr execute(AtomSpace* as, bool silent) {
		return
			CastToValue(EvaluationLink::do_evaluate(as, this->get_handle(), silent));
	}

	static Handle factory(const Handle&);
};

typedef std::shared_ptr<EvaluatableLink> EvaluatableLinkPtr;
static inline EvaluatableLinkPtr EvaluatableLinkCast(const Handle& h)
   { AtomPtr a(h); return std::dynamic_pointer_cast<EvaluatableLink>(a); }
static inline EvaluatableLinkPtr EvaluatableLinkCast(AtomPtr a)
   { return std::dynamic_pointer_cast<EvaluatableLink>(a); }

#define createEvaluatableLink std::make_shared<EvaluatableLink>

/** @}*/
}

#endif // _OPENCOG_EVALUATABLE_LINK_H
