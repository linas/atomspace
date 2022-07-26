/*
 * opencog/atoms/reduct/BoolLink.h
 *
 * Copyright (C) 2015,2018,2022 Linas Vepstas
 * All Rights Reserved
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef _OPENCOG_BOOL_LINK_H
#define _OPENCOG_BOOL_LINK_H

#include <opencog/atoms/base/Link.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * The BoolLink implements the logical operators on vectors of bools.
 * OrderedLink version.
 */
class BoolLink : public Link
{
protected:
	void init(void);

public:
	BoolLink(const HandleSeq&&, Type);

	BoolLink(const BoolLink&) = delete;
	BoolLink& operator=(const BoolLink&) = delete;

	virtual bool is_executable() const { return true; }
	virtual ValuePtr execute(AtomSpace*, bool);

	static Handle factory(const Handle&);
};

LINK_PTR_DECL(BoolLink)
#define createBoolLink CREATE_DECL(BoolLink)

/** @}*/
}

#endif // _OPENCOG_BOOL_LINK_H
