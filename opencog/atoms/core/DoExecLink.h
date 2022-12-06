/*
 * opencog/atoms/core/DoExecLink.h
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

#ifndef _OPENCOG_DO_EXEC_LINK_H
#define _OPENCOG_DO_EXEC_LINK_H

#include <opencog/atoms/base/Link.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/// The DoExecLink passes on a call to it's execute() method to 
/// everything it wraps, and then returns the result, wrapped the
/// same way.
///
class DoExecLink : public Link
{
public:
	DoExecLink(const HandleSeq&&, Type=DO_EXEC_LINK);
	DoExecLink(const DoExecLink &) = delete;
	DoExecLink& operator=(const DoExecLink &) = delete;

	virtual bool is_executable() const { return true; }
	virtual ValuePtr execute(AtomSpace*, bool);

	static Handle factory(const Handle&);
};

LINK_PTR_DECL(DoExecLink)
#define createDoExecLink CREATE_DECL(DoExecLink)

/** @}*/
}

#endif // _OPENCOG_DO_EXEC_LINK_H
