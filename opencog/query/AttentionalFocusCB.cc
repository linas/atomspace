/*
 * AttentionalFocusCB.cc
 *
 * Copyright (C) 2014 Misgana Bayetta
 *
 * Author: Misgana Bayetta <misgana.bayetta@gmail.com>  July 2014
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
#include "AttentionalFocusCB.h"

using namespace opencog;

AttentionalFocusCB::AttentionalFocusCB(AtomSpace* as) :
	DefaultPatternMatchCB(as)
{
	// Temporarily disable the AF mechanism during the URE development
	// _as->setAttentionalFocusBoundary(AttentionValue::MINSTI);
}

bool AttentionalFocusCB::node_match(const Handle& node1, const Handle& node2)
{
	return node1 == node2 and
		node2->getSTI() > _as->get_attentional_focus_boundary();
}

bool AttentionalFocusCB::link_match(const Handle& lpat, const Handle& lsoln)
{
	return DefaultPatternMatchCB::link_match(lpat, lsoln)
		and lsoln->getSTI() > _as->get_attentional_focus_boundary();
}

static bool compare_sti(const LinkPtr& lptr1, const LinkPtr& lptr2)
{
	return lptr1->getSTI() > lptr2->getSTI();
}

IncomingSet AttentionalFocusCB::get_incoming_set(const Handle& h)
{
	const IncomingSet &incoming_set = h->getIncomingSet();

	// Discard the part of the incoming set that is below the
	// AF boundary.  The PM will look only at those links that
	// this callback returns; thus we avoid searching the low-AF
	// parts of the hypergraph.
	IncomingSet filtered_set;
	for (const auto& l : incoming_set)
		if (l->getSTI() > _as->get_attentional_focus_boundary())
			filtered_set.push_back(l);

	// If nothing is in AF
	if (filtered_set.empty())
	{
		// Returning the empty set abandons the search in this direction.
		// Search will then backtrack and try a different direction.
		// ... and that is exactly what should be happening.
		return filtered_set;
	}

	// The exploration of the set of patterns proceeds by going through
	// the incoming set, one by one.  So sorting the incoming set will
	// cause the exploration to look at the highest STI atoms first.
	std::sort(filtered_set.begin(), filtered_set.end(), compare_sti);

	return filtered_set;
}
