/*
 * SplitLink.cc
 *
 * Copyright (C) 2015, 2022, 2024 Linas Vepstas
 *
 * Author: Linas Vepstas <linasvepstas@gmail.com>  January 2009
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the
 * exceptions at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <opencog/atomspace/AtomSpace.h>
#include <opencog/atoms/value/LinkValue.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/ValueFactory.h>

#include "SplitLink.h"

using namespace opencog;

SplitLink::SplitLink(const HandleSeq&& oset, Type t)
	: CollectionOfLink(std::move(oset), t)
{
	if (not nameserver().isA(t, SPLIT_LINK))
	{
		const std::string& tname = nameserver().getTypeName(t);
		throw InvalidParamException(TRACE_INFO,
			"Expecting an SplitLink, got %s", tname.c_str());
	}

	if (not _have_typespec)
	{
		_out_type = LINK_VALUE;
		_out_is_link = false;
	}

	// Split on whitespace
	_sep = " \t\n\r\v";
}

// ---------------------------------------------------------------

ValuePtr SplitLink::rewrap_h(AtomSpace* as, const Handle& base)
{
	// We could flatten lists of Nodes, and then tokenize those,
	// but right now, I'm feeling lazy.
	if (not base->is_node())
		throw RuntimeException(TRACE_INFO, "Not implemented!");

	Type ntype = base->get_type();
	HandleSeq hsq;

	const std::string& name = base->get_name();
	size_t pos = 0;
	while (true) {
		size_t prev = pos;
		pos = name.find_first_of(_sep, pos);
		if (0 < pos-prev)
		{
			const std::string& subby(name.substr(prev, pos-prev));
			if (0 < subby.length())
				hsq.emplace_back(as->add_node(ntype, std::string(subby)));
		}
		if (name.npos == pos) break;
		pos++;
	}

	if (_out_is_link)
		return as->add_link(_out_type, std::move(hsq));
	return valueserver().create(_out_type, std::move(hsq));
}

// ---------------------------------------------------------------

ValuePtr SplitLink::rewrap_v(AtomSpace* as, const ValuePtr& vp)
{
	if (vp->is_type(LINK_VALUE))
	{
		LinkValuePtr lvp(LinkValueCast(vp));
		const ValueSeq& lvsq(lvp->value());
		if (lvsq.size() == 1)
			return rewrap_v(as, lvsq[0]);

		ValueSeq vsq;
		for (const ValuePtr& lvo : lvsq)
			vsq.push_back(rewrap_v(as, lvo));

		return valueserver().create(_out_type, std::move(vsq));
	}

	if (not vp->is_type(STRING_VALUE))
		throw RuntimeException(TRACE_INFO,
			"Expecting StringValue, got %s", vp->to_string().c_str());

	ValueSeq vsq;

	// StringValues hold vectors of strings.
	StringValuePtr svp(StringValueCast(vp));
	for (const std::string& name : svp->value())
	{
		size_t pos = 0;
		while (true) {
			size_t prev = pos;
			pos = name.find_first_of(_sep, pos);
			if (0 < pos-prev)
			{
				const std::string& subby(name.substr(prev, pos-prev));
				if (0 < subby.length())
					vsq.emplace_back(createStringValue(subby));
			}
			if (name.npos == pos) break;
			pos++;
		}
	}

	return valueserver().create(_out_type, std::move(vsq));
}

// ---------------------------------------------------------------

DEFINE_LINK_FACTORY(SplitLink, SPLIT_LINK)

/* ===================== END OF FILE ===================== */
