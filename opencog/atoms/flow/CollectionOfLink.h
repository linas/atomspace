/*
 * opencog/atoms/flow/CollectionOfLink.h
 *
 * Copyright (C) 2015, 2022 Linas Vepstas
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

#ifndef _OPENCOG_COLLECTION_OF_LINK_H
#define _OPENCOG_COLLECTION_OF_LINK_H

#include <opencog/atoms/core/FunctionLink.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/// The CollectionOfLink performs a conversion between different
/// Link types and LinkValues.
///
/// In the default mode, it returns a SetLink holding the contents
/// of a LinkValue that resulted from the execution of whatever was
/// wrapped. Thus, if a pipeline produces a LinkValue, and you
/// wanted a SetLink instead, just wrap it with this.
///
/// This is little more than a simplified, dumed-down API for a one-shot
/// FilterLink, which recognizes one kind of input, and creates another.
/// The FilterLink that does this si sufficiently complicated, and this
/// is sufficiently easy, that it seems worth it.
///
class CollectionOfLink : public FunctionLink
{
protected:
	// Only simple types are handled; perhaps complex types could
	// be added someday?
	Type _out_type;
	bool _out_is_link;
	bool _have_typespec;

	void check_typespec(void);

	virtual ValuePtr rewrap_h(AtomSpace*, const Handle&);
	virtual ValuePtr rewrap_v(AtomSpace*, const ValuePtr&);

public:
	CollectionOfLink(const HandleSeq&&, Type = COLLECTION_OF_LINK);
	CollectionOfLink(const CollectionOfLink&) = delete;
	CollectionOfLink& operator=(const CollectionOfLink&) = delete;

	// Return a SetLink
	virtual ValuePtr execute(AtomSpace*, bool);

	static Handle factory(const Handle&);
};

LINK_PTR_DECL(CollectionOfLink)
#define createCollectionOfLink CREATE_DECL(CollectionOfLink)

/** @}*/
}

#endif // _OPENCOG_COLLECTION_OF_LINK_H
