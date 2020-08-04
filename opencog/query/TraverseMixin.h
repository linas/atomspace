/*
 * TraverseMixin.h
 *
 * Copyright (C) 2009, 2014, 2015 Linas Vepstas <linasvepstas@gmail.com>
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
 *
 * Created by Linas Vepstas January 2009
 */

#ifndef _OPENCOG_TRAVERSE_MIXIN_H
#define _OPENCOG_TRAVERSE_MIXIN_H

#include <opencog/atomspace/AtomSpace.h>
#include <opencog/query/PatternMatchCallback.h>

namespace opencog {

/**
 * Callback mixin class, used to provide a default incoming-set
 * traversal.
 */
class TraverseMixin : public virtual PatternMatchCallback
{
	public:
		TraverseMixin(AtomSpace*);
		virtual ~TraverseMixin();
		virtual IncomingSet get_incoming_set(const Handle&, Type);

	protected:
		AtomSpace* _as;
};

} // namespace opencog

#endif // _OPENCOG_TRAVERSE_MIXIN_H
