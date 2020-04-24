/*
 * opencog/atoms/reduct/GreaterThanLink.h
 *
 * Copyright (C) 2020 Linas Vepstas
 * All Rights Reserved
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef _OPENCOG_GREATER_THAN_LINK_H
#define _OPENCOG_GREATER_THAN_LINK_H

#include <opencog/atoms/core/FunctionLink.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * The GreaterThanLink implements the arithmetic operation of "greater
 * than" on a component-by-component level. That is,
 *    GreaterThan (a, b, c) (d, e, f) is just (a>d,  b>e, c>f).
 */
class GreaterThanLink : public FunctionLink
{
protected:
	void init(void);

public:
	GreaterThanLink(const Handle& a);
	GreaterThanLink(const Handle& a, const Handle& b);
	GreaterThanLink(const HandleSeq&&, Type=GREATER_THAN_LINK);

	GreaterThanLink(const GreaterThanLink&) = delete;
	GreaterThanLink& operator=(const GreaterThanLink&) = delete;

	virtual ValuePtr execute(AtomSpace*, bool);

	static Handle factory(const Handle&);
};

typedef std::shared_ptr<GreaterThanLink> GreaterThanLinkPtr;
static inline GreaterThanLinkPtr GreaterThanLinkCast(const Handle& h)
   { AtomPtr a(h); return std::dynamic_pointer_cast<GreaterThanLink>(a); }
static inline GreaterThanLinkPtr GreaterThanLinkCast(AtomPtr a)
   { return std::dynamic_pointer_cast<GreaterThanLink>(a); }

#define createGreaterThanLink std::make_shared<GreaterThanLink>

/** @}*/
}

#endif // _OPENCOG_GREATER_THAN_LINK_H
