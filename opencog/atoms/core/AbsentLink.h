/*
 * opencog/atoms/core/AbsentLink.h
 *
 * Copyright (C) 2015,2019 Linas Vepstas
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

#ifndef _OPENCOG_ABSENT_LINK_H
#define _OPENCOG_ABSENT_LINK_H

#include <opencog/atoms/base/Link.h>
#include <opencog/atoms/core/FreeLink.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/// The AbsentLink is used to determine whether or not Atoms of a given
/// form exist, or not, in the AtomSpace.
///
/// The AbsentLink, when written with VariableNodes in it, is just
/// another atom. However, an AbsentLink that does NOT have any
/// VariableNodes (or GlobNodes) within it is NOT allowed to be placed
/// into any atomspace.
///
/// In essence, the AbsentLink is a link that can never be grounded!
/// In thise sense, it is similar to the DeleteLink, except that,
/// instead of tirggering deletion, it just answers a simple yes/no
/// question as to whether the grounded form exists in an AtomSpace.
///
class AbsentLink : public FreeLink
{
protected:
	void init(void);
	void setAtomSpace(AtomSpace *);
public:
	AbsentLink(const HandleSeq&, Type=ABSENT_LINK);

	AbsentLink(const Link&);

	virtual TruthValuePtr evaluate(AtomSpace*, bool);
	static Handle factory(const Handle&);
};

typedef std::shared_ptr<AbsentLink> AbsentLinkPtr;
static inline AbsentLinkPtr AbsentLinkCast(const Handle& h)
	{ return std::dynamic_pointer_cast<AbsentLink>(h); }
static inline AbsentLinkPtr AbsentLinkCast(const AtomPtr& a)
	{ return std::dynamic_pointer_cast<AbsentLink>(a); }

#define createAbsentLink std::make_shared<AbsentLink>

/** @}*/
}

#endif // _OPENCOG_ABSENT_LINK_H
