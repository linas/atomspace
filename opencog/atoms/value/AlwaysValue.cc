/*
 * opencog/atoms/value/AlwaysValue.cc
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

#include <opencog/atoms/value/AlwaysValue.h>
#include <opencog/atoms/value/ValueFactory.h>

using namespace opencog;

// ==============================================================

AlwaysValue::AlwaysValue(const Handle& h)
	: GroupValue(h)
{
	_type = ALWAYS_VALUE;
}

AlwaysValue::~AlwaysValue()
{
}

// ==============================================================

void AlwaysValue::add(const ValuePtr& vp)
{
	add(ValuePtr(vp));
}

/// Add one item to the container. All items must be equivalent to
/// each other. If an attempt is made to add a non-equivalent item,
/// the container is cleared and closed.
void AlwaysValue::add(ValuePtr&& vp)
{
	// Already closed (either normally or due to failure) - ignore
	if (is_closed()) return;

	// VoidValue or empty LinkValue signals end-of-stream.
	if ((vp->get_type() == VOID_VALUE) or
	    (vp->is_type(LINK_VALUE) and 0 == vp->size()))
	{
		close();
		return;
	}

	// First item - just add it directly
	ValuePtr rep = peek();
	if (nullptr == rep)
	{
		_set.insert(std::move(vp));
		return;
	}

	_scratch->clear();

	// Check equivalence against an existing item
	if (equivalent(*vp, *rep))
	{
		// Equivalent - add it
		_set.insert(std::move(vp));
	}
	else
	{
		// Non-equivalent item - fail!
		_set.clear();
		close();
	}
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_VALUE_FACTORY(ALWAYS_VALUE, createAlwaysValue, const Handle&)
