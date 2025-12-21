/*
 * opencog/atoms/value/MonoValue.cc
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

#include <opencog/atoms/value/MonoValue.h>
#include <opencog/atoms/value/ValueFactory.h>

using namespace opencog;

// ==============================================================

MonoValue::MonoValue(const Handle& h)
	: GroupValue(h)
{
	_type = MONO_VALUE;
}

MonoValue::~MonoValue()
{
}

// ==============================================================

void MonoValue::add(const ValuePtr& vp)
{
	add(ValuePtr(vp));
}

/// Add item to the container. Must be equivalent to existing items.
/// If an attempt is made to add a non-equivalent item, the container
/// is cleared and closed.
void MonoValue::add(ValuePtr&& vp)
{
	if (is_closed()) return;

	// Empty Values signal end-of-stream.
	if (0 == vp->size())
	{
		close();
		return;
	}

	// Atomic insert into possibly empty set.
	std::optional<ValuePtr> rep = _set.try_insert(std::move(vp));
	if (not rep.has_value())
		return;  // First item was inserted

	// Set was non-empty. Check equivalence against existing item.
	// Note: vp was NOT moved (try_insert didn't insert it), so still valid.
	_scratch->clear();

	if (equivalent(*vp, *rep.value()))
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
DEFINE_VALUE_FACTORY(MONO_VALUE, createMonoValue, const Handle&)
