/*
 * opencog/atoms/base/Link.cc
 *
 * Copyright (C) 2008-2010 OpenCog Foundation
 * Copyright (C) 2002-2007 Novamente LLC
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

#include <stdio.h>

#include <opencog/util/exceptions.h>
#include <opencog/util/Logger.h>

#include <opencog/atoms/atom_types/NameServer.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atomspace/AtomTable.h>

#include "Link.h"

//#define DPRINTF printf
#define DPRINTF(...)

using namespace opencog;

void Link::init(const HandleSeq& outgoingVector)
{
    if (not nameserver().isA(_type, LINK)) {
        throw InvalidParamException(TRACE_INFO,
            "Link ctor: Atom type is not a Link: '%d' %s.",
            _type, nameserver().getTypeName(_type).c_str());
    }

    // Yes, people actually send us bad data.
    for (const Handle& h: outgoingVector)
        if (nullptr == h)
            throw InvalidParamException(TRACE_INFO,
                "Link ctor: invalid outgoing set!");

    _outgoing = outgoingVector;
}

void Link::init(const HandleSeq&& outgoingVector)
{
    if (not nameserver().isA(_type, LINK)) {
        throw InvalidParamException(TRACE_INFO,
            "Link ctor: Atom type is not a Link: '%d' %s.",
            _type, nameserver().getTypeName(_type).c_str());
    }

    // Yes, people actually send us bad data.
    for (const Handle& h: outgoingVector)
        if (nullptr == h)
            throw InvalidParamException(TRACE_INFO,
                "Link ctor: invalid outgoing set!");

    _outgoing = std::move(outgoingVector);
}

Link::~Link()
{
    DPRINTF("Deleting link:\n%s\n", this->to_string().c_str());
}

/// Return a universally-unique string for each distinct link.
/// It needs to be fast, to be human-readable, and without any
/// trailing newlines.
std::string Link::to_short_string(const std::string& indent) const
{
    std::stringstream answer;
    std::string more_indent = indent + "  ";

    answer << indent << "(" << nameserver().getTypeName(_type);

    // Here the target string is made. If a target is a node, its name is
    // concatenated. If it's a link, all its properties are concatenated.
    for (const Handle& h : _outgoing)
        answer << " " << h->to_short_string();

    answer << indent << ")";

    return answer.str();
}

std::string Link::to_string(const std::string& indent) const
{
    std::string answer = indent;
    std::string more_indent = indent + "  ";

    answer += "(" + nameserver().getTypeName(_type);

    // Print the TV only if its not the default.
    if (not getTruthValue()->isDefaultTV())
        answer += " " + getTruthValue()->to_string();

    answer += "\n";
    // Here, the outset string is made. If a target is a node,
    // its name is concatenated. If it's a link, then recurse.
    for (const Handle& h : _outgoing)
        answer += h->to_string(more_indent) + "\n";

    answer += indent + ") ; " + id_to_string();

    return answer;
}

/// Content-based comparison.
bool Link::operator==(const Atom& other) const
{
    // If other points to this, then have equality.
    if (this == &other) return true;

    if (get_type() != other.get_type()) return false;

    // Rule out obvious mis-matches, based on the hash.
    if (get_hash() != other.get_hash()) return false;

    Arity sz = get_arity();
    if (sz != other.get_arity()) return false;

    // Perform a content-compare on the outgoing set.
    const HandleSeq& rhs = other.getOutgoingSet();
    for (Arity i = 0; i < sz; i++)
    {
        if (*((AtomPtr)_outgoing[i]) != *((AtomPtr)rhs[i]))
            return false;
    }
    return true;
}

/// Content-based comparison.
bool Link::equal(Type t, const HandleSeq& oset) const
{
    if (get_type() != t) return false;

    Arity sz = get_arity();
    if (sz != oset.size()) return false;

    // Perform a content-compare on the outgoing set.
    for (Arity i = 0; i < sz; i++)
    {
        if (*((AtomPtr)_outgoing[i]) != *((AtomPtr)oset[i]))
            return false;
    }
    return true;
}

// Content-based ordering.
bool Link::operator<(const Atom& other) const
{
    if (this == &other) return false;

    ContentHash cht = get_hash();
    ContentHash cho = other.get_hash();
    if (cht != cho) return cht < cho;

    // We get to here only if the hashes are equal.
    // Compare the contents directly, for this
    // (hopefully rare) case.
    if (get_type() != other.get_type())
        return get_type() < other.get_type();

    const HandleSeq& outgoing = getOutgoingSet();
    const HandleSeq& other_outgoing = other.getOutgoingSet();
    Arity arity = outgoing.size();
    Arity other_arity = other_outgoing.size();
    if (arity != other_arity)
        return arity < other_arity;

    // Before comparing their outgoings, make sure that they are not
    // equal. Indeed, in some cases, such as alpha-equivalence,
    // comparing their outgoings won't work
    if (operator==(other))
	    return false;

    for (Arity i=0; i < arity; i++)
    {
        const Handle& ll(outgoing[i]);
        const AtomPtr& rl(other_outgoing[i]);
        if (ll->operator!=(*rl))
            return ll->operator<(*rl);
    }
    return false;
}

/// Returns a Merkle tree hash -- that is, the hash of this link
/// chains the hash values of the child atoms, as well.
ContentHash Link::compute_hash(Type type, const HandleSeq& oset)
{
	// 1<<44 - 377 is prime
	ContentHash hsh = ((1UL<<44) - 377) * type;
	for (const Handle& h: oset)
	{
		hsh += (hsh <<5) ^ (353 * h->get_hash()); // recursive!

		// Bit-mixing copied from murmur64. Yes, this is needed.
		hsh ^= hsh >> 33;
		hsh *= 0xff51afd7ed558ccdL;
		hsh ^= hsh >> 33;
		hsh *= 0xc4ceb9fe1a85ec53L;
		hsh ^= hsh >> 33;
	}

	// Links will always have the MSB set.
	ContentHash mask = ((ContentHash) 1UL) << (8*sizeof(ContentHash) - 1);
	hsh |= mask;

	if (Handle::INVALID_HASH == hsh) hsh -= 1;
	return hsh;
}

ContentHash Link::compute_hash() const
{
	_content_hash = compute_hash(get_type(), _outgoing);
	return _content_hash;
}

/// Place `this` into the incoming set of each outgoing atom.
///
void Link::install()
{
	LinkPtr llc(LinkCast(get_handle()));
	size_t arity = get_arity();
	for (size_t i = 0; i < arity; i++)
		_outgoing[i]->insert_atom(llc);
}

void Link::remove()
{
	LinkPtr lll(LinkCast(get_handle()));
	size_t arity = get_arity();
	for (size_t i = 0; i < arity; i++)
		_outgoing[i]->remove_atom(lll);
}
