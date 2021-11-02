/*
 * opencog/atoms/pattern/ChainLink.h
 *
 * Copyright (C) 2021 Linas Vepstas
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

#ifndef _OPENCOG_CHAIN_LINK_H
#define _OPENCOG_CHAIN_LINK_H

#include <opencog/atoms/pattern/PatternLink.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/// The ChainLink is an experimental pattern-matching link that enables
/// (recursive) chaining. It resembles a VritualLink (a relation), in
/// that it accepts a list of arguments, and evaluates to something true
/// or false, based on those arguments.  However, this is not how it
/// would normally be used. Instead, the typical use would be one where
/// only some of its arguments are provided. Given these, it is entered,
/// and then runs like a conventional PatternLink, and tries to ground
/// the rest of it's arguments.
///
/// Upon finding some additional groundings, it then returns with the
/// first of those groundings. On subsequent calls, it will provide
/// additional groundings, and thus operates like any of the other
/// branch points in the pattern engine.
///
class ChainLink : public PatternLink
{
public:
	ChainLink(const HandleSeq&&, Type=CHAIN_LINK);
	ChainLink(const Handle& varcdecls, const Handle& body);
	ChainLink(const ChainLink &) = delete;
	ChainLink& operator=(const ChainLink &) = delete;

	static Handle factory(const Handle&);
};

typedef std::shared_ptr<ChainLink> ChainLinkPtr;
static inline ChainLinkPtr ChainLinkCast(const Handle& h)
	{ return std::dynamic_pointer_cast<ChainLink>(h); }
static inline ChainLinkPtr ChainLinkCast(const AtomPtr& a)
	{ return std::dynamic_pointer_cast<ChainLink>(a); }

#define createChainLink std::make_shared<ChainLink>

/** @}*/
}

#endif // _OPENCOG_CHAIN_LINK_H
