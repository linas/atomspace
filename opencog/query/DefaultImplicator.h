/*
 * DefaultImplicator.h
 *
 * Copyright (C) 2009, 2014 Linas Vepstas
 *
 * Author: Linas Vepstas <linasvepstas@gmail.com>  January 2009
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

#ifndef _OPENCOG_DEFAULT_IMPLICATOR_H
#define _OPENCOG_DEFAULT_IMPLICATOR_H

#include "DefaultPatternMatchCB.h"
#include "Implicator.h"
#include "InitiateSearchCB.h"
#include "PatternMatchCallback.h"


namespace opencog {

// Flirting with diamond-pattern inheritance. We need to do this
// because we want different helper classes (aka "mixin classes")
// to handle different parts of the grounding process, and virtual
// bases classes just minimizes the total boilerplate code needed.
class DefaultImplicator:
	public virtual Implicator,
	public virtual InitiateSearchCB,
	public virtual DefaultPatternMatchCB
{
	public:
		DefaultImplicator(AtomSpace* asp) :
			Implicator(asp),
			InitiateSearchCB(asp),
			DefaultPatternMatchCB(asp) {}

	virtual void set_pattern(const Variables& vars,
	                         const Pattern& pat)
	{
		InitiateSearchCB::set_pattern(vars, pat);
		DefaultPatternMatchCB::set_pattern(vars, pat);
	}
};

}; // namespace opencog

#endif // _OPENCOG_DEFAULT_IMPLICATOR_H
