/*
 * opencog/atoms/execution/ExecutableLink.h
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

#ifndef _OPENCOG_EXECUTABLE_LINK_H
#define _OPENCOG_EXECUTABLE_LINK_H

#include <opencog/atoms/base/Link.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

class AtomSpace;

class ExecutableLink
{
public:
	ExecutableLink() {}
	virtual Handle execute(AtomSpace*, bool silent=false) const = 0;
};

typedef ExecutableLink* ExecutableLinkPtr;
static inline ExecutableLinkPtr ExecutableLinkCast(const Handle& h)
   { return reinterpret_cast<ExecutableLink*>(h.get()); }
static inline ExecutableLinkPtr ExecutableLinkCast(AtomPtr a)
   { return reinterpret_cast<ExecutableLink*>(a.get()); }

/** @}*/
}

#endif // _OPENCOG_EXECUTABLE_LINK_H
