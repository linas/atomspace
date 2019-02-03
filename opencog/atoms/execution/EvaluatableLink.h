/*
 * opencog/atoms/execution/EvaluatableLink.h
 *
 * Copyright (C) 2013,2014,2015,2019 Linas Vepstas
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

#ifndef _OPENCOG_EVALUATABLE_LINK_H
#define _OPENCOG_EVALUATABLE_LINK_H

#include <opencog/atoms/base/Link.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

class AtomSpace;

class EvaluatableLink
{
public:
	EvaluatableLink() {}
	virtual TruthValuePtr evaluate(AtomSpace*) const = 0;
};

typedef EvaluatableLink* EvaluatableLinkPtr;
static inline EvaluatableLinkPtr EvaluatableLinkCast(const Handle& h)
   { return reinterpret_cast<EvaluatableLink*>(h.get()); }
static inline EvaluatableLinkPtr EvaluatableLinkCast(AtomPtr a)
   { return reinterpret_cast<EvaluatableLink*>(a.get()); }

/** @}*/
}

#endif // _OPENCOG_EVALUATABLE_LINK_H
