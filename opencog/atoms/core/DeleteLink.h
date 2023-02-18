/*
 * opencog/atoms/core/DeleteLink.h
 *
 * Copyright (C) 2015 Linas Vepstas
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

#ifndef _OPENCOG_DELETE_LINK_H
#define _OPENCOG_DELETE_LINK_H

#include <opencog/atoms/base/Link.h>
#include <opencog/atoms/core/FreeLink.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/// The DeleteLink will delete itself and it's contents, when executed.
/// It is intended to be used in execution contexts, where one or more
/// Atoms have been found, and need to be deleted.
///
/// DeleteLinks with free variables in them will not self-delete. The
/// goal here is to allow DeleteLinks to be used in queries and other
/// contexts, where the variables are place-holders.
///
/// When executed, a DeleteLink will perform a recursuve delete of it's
/// contents. That is, any other Links that wrap the contents will also
/// be deleted.
///
class DeleteLink : public FreeLink
{
protected:
	void init(void);
public:
	DeleteLink(const HandleSeq&&, Type=DELETE_LINK);

	DeleteLink(const DeleteLink&) = delete;
	DeleteLink& operator=(const DeleteLink&) = delete;

	virtual bool is_executable() const { return true; }
	virtual ValuePtr execute(AtomSpace*, bool);

	static Handle factory(const Handle&);
};

LINK_PTR_DECL(DeleteLink)
#define createDeleteLink CREATE_DECL(DeleteLink)

/** @}*/
}

#endif // _OPENCOG_DELETE_LINK_H
