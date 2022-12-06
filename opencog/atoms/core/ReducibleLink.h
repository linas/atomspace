/*
 * opencog/atoms/core/ReducibleLink.h
 *
 * Copyright (C) 2017, 2022 Linas Vepstas
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

#ifndef _OPENCOG_REDUCIBLE_LINK_H
#define _OPENCOG_REDUCIBLE_LINK_H

#include <opencog/atoms/base/Link.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/// The ReducibleLink passes on a call to it's execute() method to 
/// everything it wraps, and then returns the result, wrapped the
/// same way.
///
class ReducibleLink : public Link
{
public:
	ReducibleLink(const HandleSeq&&, Type=REDUCIBLE_LINK);
	ReducibleLink(const ReducibleLink &) = delete;
	ReducibleLink& operator=(const ReducibleLink &) = delete;

	virtual bool is_executable() const { return true; }
	virtual ValuePtr execute(AtomSpace*, bool);

	static Handle factory(const Handle&);
};

LINK_PTR_DECL(ReducibleLink)
#define createReducibleLink CREATE_DECL(ReducibleLink)

/** @}*/
}

#endif // _OPENCOG_REDUCIBLE_LINK_H
