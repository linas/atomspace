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

#ifndef _OPENCOG_EVALUTATABLE_LINK_H
#define _OPENCOG_EVALUTATABLE_LINK_H

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
	virtual TruthValuePtr evaluate(AtomSpace*) = 0;
};

typedef EvaluatableLink* EvaluatableLinkPtr;
static inline EvaluatableLinkPtr EvaluatableLinkCast(const Handle& h)
   { return (EvaluatableLink*) h.get(); }
   // { return std::dynamic_cast<EvaluatableLink*>(h.get()); }
   // { return std::dynamic_pointer_cast<EvaluatableLink>(h); }
static inline EvaluatableLinkPtr EvaluatableLinkCast(AtomPtr a)
   { return (EvaluatableLink*) a.get(); }
   // { return std::dynamic_cast<EvaluatableLink*>(a.get()); }

/** @}*/
}

#endif // _OPENCOG_EVALUTATABLE_LINK_H
