/*
 * opencog/atoms/reduct/SineLink.h
 *
 * Copyright (C) 2020,2022 Linas Vepstas
 * All Rights Reserved
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef _OPENCOG_SINE_LINK_H
#define _OPENCOG_SINE_LINK_H

#include <opencog/atoms/reduct/NumericFunctionLink.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * The SineLink implements the trigonometric sine.
 */
class SineLink : public NumericFunctionLink
{
protected:
	void init(void);

public:
	SineLink(const Handle& a);
	SineLink(const HandleSeq&&, Type=SINE_LINK);

	SineLink(const SineLink&) = delete;
	SineLink& operator=(const SineLink&) = delete;

	virtual ValuePtr execute(AtomSpace*, bool);

	static Handle factory(const Handle&);
};

LINK_PTR_DECL(SineLink)
#define createSineLink CREATE_DECL(SineLink)

/** @}*/
}

#endif // _OPENCOG_SINE_LINK_H
