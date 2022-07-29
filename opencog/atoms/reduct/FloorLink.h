/*
 * opencog/atoms/reduct/FloorLink.h
 *
 * Copyright (C) 2020,2022 Linas Vepstas
 * All Rights Reserved
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef _OPENCOG_FLOOR_LINK_H
#define _OPENCOG_FLOOR_LINK_H

#include <opencog/atoms/reduct/NumericFunctionLink.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * The FloorLink truncates floats, returning the integer part.
 * It "rounds down", to the floor.
 */
class FloorLink : public NumericFunctionLink
{
protected:
	void init(void);

public:
	FloorLink(const Handle& a);
	FloorLink(const HandleSeq&&, Type=FLOOR_LINK);

	FloorLink(const FloorLink&) = delete;
	FloorLink& operator=(const FloorLink&) = delete;

	virtual ValuePtr execute(AtomSpace*, bool);

	static Handle factory(const Handle&);
};

LINK_PTR_DECL(FloorLink)
#define createFloorLink CREATE_DECL(FloorLink)

/** @}*/
}

#endif // _OPENCOG_FLOOR_LINK_H
