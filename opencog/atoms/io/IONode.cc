/*
 * opencog/atoms/io/IONode.cc
 *
 * Copyright (c) 2008-2010 OpenCog Foundation
 * Copyright (c) 2009,2013,2020,2022 Linas Vepstas
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

#include <string>

#include <opencog/atomspace/AtomSpace.h>
#include "IONode.h"

using namespace opencog;

// ====================================================================

IONode::IONode(Type t, std::string uri) :
	Node(t, uri)
{
	if (not nameserver().isA(t, I_O_NODE))
		throw RuntimeException(TRACE_INFO, "Bad inheritance!");
}

IONode::~IONode()
{
}

void IONode::create(void)
{
	throw RuntimeException(TRACE_INFO, "Not Implemented!");
}

void IONode::destroy(void)
{
	throw RuntimeException(TRACE_INFO, "Not Implemented!");
}

void IONode::erase(void)
{
	throw RuntimeException(TRACE_INFO, "Not Implemented!");
}

std::string IONode::monitor(void)
{
	return "This IONode does not implement a monitor.";
}

// ====================================================================

void IONode::barrier(AtomSpace* as)
{
	if (nullptr == as) as = getAtomSpace();
	as->barrier();
}

// ====================================================================
