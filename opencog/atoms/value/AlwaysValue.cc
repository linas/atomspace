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

/// Add item to the container. Must be equivalent to existing items.
/// If an attempt is made to add a non-equivalent item, the container
/// is cleared and closed.
void AlwaysValue::add(ValuePtr&& vp)
{
	if (is_closed()) return;

	// Empty Values signal end-of-stream.
	if (0 == vp->size()))
	{
		close();
		return;
	}

	ValuePtr rep = peek();
	if (nullptr == rep)
	{
		_set.insert(std::move(vp));
		return;
	}

	_scratch->clear();

	if (equivalent(*vp, *rep))
	{
		_set.insert(std::move(vp));
		return;
	}

	// Non-equivalent item - fail!
	_set.clear();
	close();
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_VALUE_FACTORY(ALWAYS_VALUE, createAlwaysValue, const Handle&)
