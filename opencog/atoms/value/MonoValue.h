/*
 * opencog/atoms/value/AlwaysValue.h
 *
 * Copyright (C) 2025 BrainyBlaze Dynamics, LLC
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _OPENCOG_ALWAYS_VALUE_H
#define _OPENCOG_ALWAYS_VALUE_H

#include <opencog/atoms/value/GroupValue.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * A ContainerValue that enforces universal equivalence across all items.
 * It is similar to GroupValue, except it allows only a single bucket.
 * Items added to AlwaysValue are checked for equivalence against existing
 * items. If an attempt is made to add an item that is not equivalent to
 * all the others, the entire set accumulated so far is cleared, and the
 * AlwaysValue is closed (finishing in a closed and empty state).
 *
 * The equivalence relation is the same as for GroupValue: any Atomese
 * function that evaluates to crisp true/false when given a pair of
 * input values.
 */
class AlwaysValue
	: public GroupValue
{
public:
	AlwaysValue(const Handle&);
	virtual ~AlwaysValue();

	virtual void add(const ValuePtr&) override;
	virtual void add(ValuePtr&&) override;
};

VALUE_PTR_DECL(AlwaysValue);
CREATE_VALUE_DECL(AlwaysValue);

/** @}*/
} // namespace opencog

#endif // _OPENCOG_ALWAYS_VALUE_H
