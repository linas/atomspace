/*
 * opencog/atoms/truthvalue/AtomTruth.cc
 *
 * Copyright (C) 2000-2017 OpenCog Foundation
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

#ifndef WIN32
#include <unistd.h>
#endif

#include <opencog/util/misc.h>
#include <opencog/util/platform.h>

#include <opencog/atoms/base/Atom.h>
#include <opencog/atoms/base/Handle.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atomspace/AtomSpace.h>  // for TVChSigl

using namespace opencog;

// ==============================================================
// This implements a semi-deprecated API to truth values on Atoms.
// it's widely used, everywhere, so we cannot really eliminate it.
// However, it cannot be included in libatombase.so because it
// creates a circular shared-library dependency.  So we put it here,
// instead.

static const Handle& truth_key(void)
{
	static Handle tk(createNode(PREDICATE_NODE, "*-TruthValueKey-*"));
	return tk;
}

void Atom::setTruthValue(const TruthValuePtr& newTV)
{
    if (nullptr == newTV) return;

    // We need to guarantee that the signal goes out with the
    // correct truth value.  That is, another setter could be changing
    // this, even as we are.  So make a copy, first.
    TruthValuePtr oldTV(getTruthValue());

    // If both old and new are e.g. DEFAULT_TV, then do nothing.
    if (oldTV.get() == newTV.get()) return;

    // ... and we still need to make sure that only one thread is
    // writing this at a time. std:shared_ptr is NOT thread-safe against
    // multiple writers: see "Example 5" in
    // http://www.boost.org/doc/libs/1_53_0/libs/smart_ptr/shared_ptr.htm#ThreadSafety
    setValue (truth_key(), ProtoAtomCast(newTV));

    if (_atom_space != nullptr) {
        TVCHSigl& tvch = _atom_space->_atom_table.TVChangedSignal();
        tvch(get_handle(), oldTV, newTV);
    }
}

TruthValuePtr Atom::getTruthValue() const
{
    ProtoAtomPtr pap(getValue(truth_key()));
    if (nullptr == pap) return TruthValue::DEFAULT_TV();
    return TruthValueCast(pap);
}
