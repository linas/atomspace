/*
 * opencog/atoms/flow/ValueOfLink.h
 *
 * Copyright (C) 2018 Linas Vepstas
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _OPENCOG_VALUE_OF_LINK_H
#define _OPENCOG_VALUE_OF_LINK_H

#include <opencog/atoms/reduct/NumericOutLink.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/// The ValueOfLink returns the value on the indicated atom (first
/// argument) at the indicated key (second argument).
///
class ValueOfLink : public NumericOutLink
{
public:
	ValueOfLink(const HandleSeq&&, Type=VALUE_OF_LINK);

	ValueOfLink(const ValueOfLink&) = delete;
	ValueOfLink& operator=(const ValueOfLink&) = delete;

	// Return a pointer to the value at the specified key.
	virtual ValuePtr execute(AtomSpace*, bool);

	static Handle factory(const Handle&);
};

typedef std::shared_ptr<ValueOfLink> ValueOfLinkPtr;
static inline ValueOfLinkPtr ValueOfLinkCast(const Handle& h)
	{ return std::dynamic_pointer_cast<ValueOfLink>(h); }
static inline ValueOfLinkPtr ValueOfLinkCast(AtomPtr a)
	{ return std::dynamic_pointer_cast<ValueOfLink>(a); }

#define createValueOfLink std::make_shared<ValueOfLink>

/** @}*/
}

#endif // _OPENCOG_VALUE_OF_LINK_H
