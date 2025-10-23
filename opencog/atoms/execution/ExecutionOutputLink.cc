/*
 * opencog/atoms/execution/ExecutionOutputLink.cc
 *
 * Copyright (C) 2009, 2013, 2015 Linas Vepstas
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

#include <stdlib.h>

#include <opencog/atoms/atom_types/atom_types.h>
#include <opencog/atomspace/AtomSpace.h>
#include <opencog/atoms/core/DefineLink.h>
#include <opencog/atoms/core/LambdaLink.h>

#include "ExecutionOutputLink.h"
#include "GroundedProcedureNode.h"

using namespace opencog;

void ExecutionOutputLink::check_schema(const Handle& schema) const
{
	// Derived types do their own validation.
	if (EXECUTION_OUTPUT_LINK != get_type()) return;

	Type st = schema->get_type();
	if ((not schema->is_type(FUNCTION_LINK)) and
	    LAMBDA_LINK != st and
	    RULE_LINK != st and
	    (not schema->is_type(PROCEDURE_NODE)) and
	    // In case it is a pattern matcher query
	    VARIABLE_NODE != st and
	    UNQUOTE_LINK != st)
	{
		throw SyntaxException(TRACE_INFO,
		                      "ExecutionOutputLink must have schema! Got %s",
		                      schema->to_string().c_str());
	}
}

ExecutionOutputLink::ExecutionOutputLink(const HandleSeq&& oset, Type t)
	: FunctionLink(std::move(oset), t)
{
	if (!nameserver().isA(t, EXECUTION_OUTPUT_LINK))
		throw SyntaxException(TRACE_INFO,
		                      "Exception an ExecutionOutputLink!");

	if (2 != oset.size())
		throw SyntaxException(TRACE_INFO,
		       "ExecutionOutputLink must have schema and args! Got arity=%d",
		        oset.size());

	check_schema(oset[0]);
}

ExecutionOutputLink::ExecutionOutputLink(const Handle& schema,
                                         const Handle& args)
	: FunctionLink({schema, args}, EXECUTION_OUTPUT_LINK)
{
	check_schema(schema);
}

/// execute -- execute the function defined in an ExecutionOutputLink
///
/// Each ExecutionOutputLink should have the form:
///
///     ExecutionOutputLink
///         GroundedSchemaNode "lang: func_name"
///         ListLink
///             SomeAtom
///             OtherAtom
///
/// The "lang:" should be "scm:" for scheme, "py:" for python, or
/// "lib:" for an unix (gnu elf) shared library. See GroundedSchema
/// for details on supported formats.
///
/// This method will then invoke "func_name" on the provided ListLink
/// of arguments to the function.
///
ValuePtr ExecutionOutputLink::execute(AtomSpace* as, bool silent)
{
	ValuePtr vp(execute_once(as, silent));

	// Should never happen. But it can happen if the API implementation
	// is screwed up. Since nothing should ever be screwed up, this can't
	// happen.
	if (not vp)
		throw SyntaxException(TRACE_INFO,
			"ExecutionOutputLink: Execution gave null result: %s",
				to_string().c_str());

	if (not vp->is_atom()) return vp;

	Handle res(HandleCast(vp));
	if (not (SET_LINK == res->get_type()))
	{
		if (res->is_executable())
		{
			vp = res->execute(as, silent);
			if (not vp->is_atom()) return vp;
			res = HandleCast(vp);
		}
		return vp;
	}

	// If we are here, then its a SetLink; unwrap it, execute,
	// and re-wrap it. Basically, we distribute over the contents
	// of a Set.  This is very much like how PutLink works.
	// Is there a better way? I don't know.
	HandleSeq elts;
	for (Handle elt: res->getOutgoingSet())
	{
		if (elt->is_executable())
		{
			elt = as->add_atom(elt);
			vp = elt->execute(as, silent);
			if (not vp->is_atom()) break;
			elt = HandleCast(vp);
		}
		elts.push_back(elt);
	}

	return createLink(std::move(elts), SET_LINK);
}

/// execute_argseq -- execute a seq of arguments, return a seq of results.
///
/// Somewhat like force_execute(), but assumes that each atom knows
/// how to behave itself correctly. Much like PutLink, this also tries
/// to deal with multiple arguments that are sets (so that a SetLink
/// has the semantics of "apply to all members of the set")
static inline HandleSeq execute_argseq(AtomSpace* as, HandleSeq args,
                                       bool silent, bool& have_set)
{
	HandleSeq exargs;
	for (const Handle& h: args)
	{
		if (not h->is_executable())
		{
			// XXX should be be unwrapping SetLinks here?
			exargs.push_back(h);
			continue;
		}

		// If we are here, the argument is executable.
		ValuePtr vp = h->execute(as, silent);
		if (not vp->is_atom()) // Yuck!
			exargs.push_back(h);
		else
		{
			Handle hex(HandleCast(vp));
			if (SET_LINK == hex->get_type())
			{
				size_t sz = hex->get_arity();
				// Unwrap SetLink singletons.
				if (1 == sz)
					hex = hex->getOutgoingAtom(0);
				else if (1 < sz)
					have_set = true;
			}
			exargs.push_back(hex);
		}
	}
	return exargs;
}

ValuePtr ExecutionOutputLink::execute_once(AtomSpace* as, bool silent)
{
	Handle sn(_outgoing[0]);
	Handle args(_outgoing[1]);
	if (sn->is_type(GROUNDED_PROCEDURE_NODE))
	{
		GroundedProcedureNodePtr gsn = GroundedProcedureNodeCast(sn);
		if (nullptr == gsn)
			throw SyntaxException(TRACE_INFO,
				"ExecutionOutputLink: Cannot use naked %s",
				sn->to_string().c_str());

		return gsn->execute_args(as, args, silent);
	}

	if (sn->is_type(DEFINED_PROCEDURE_NODE))
		sn = DefineLink::get_definition(sn);

	if (sn->is_type(FUNCTION_LINK))
	{
		FunctionLinkPtr flp = FunctionLinkCast(sn);
		const FreeVariables& vars = flp->get_vars();
		const HandleSeq& oset(LIST_LINK == args->get_type() ?
			args->getOutgoingSet(): HandleSeq{args});
		Handle reduct = vars.substitute_nocheck(sn, oset, silent);
		ValuePtr vp = reduct->execute(as, silent);
		return vp;
	}

	Type st = sn->get_type();
	if (LAMBDA_LINK == st or RULE_LINK == st)
	{
		// Unpack and beta-reduce the Lambda link.
		// Execute the args before plugging them in.
		ScopeLinkPtr slp(ScopeLinkCast(sn));
		Handle body(slp->get_body());
		Variables vars(slp->get_variables());
		const HandleSeq& oset(LIST_LINK == args->get_type() ?
			args->getOutgoingSet(): HandleSeq{args});

		// If one of the arguments is a SetLink, then apply the
		// lambda expression to each of the mebers in the set.
		// This is also how PutLink works. It's needed to handle
		// the case where GetLink returns a set of multiple results;
		// we want to emulate that set passing through the processing
		// pipeline. (XXX Is there a better way of doing this?)
		// If there is more than one SetLink, then this won't work,
		// and we need to make a Cartesian product of them, instead.
		bool have_set = false;
		HandleSeq xargs(execute_argseq(as, oset, silent, have_set));

		if (not have_set)
			return as->add_atom(vars.substitute_nocheck(body, xargs));

		// Ugh. First, find the SetLink.
		size_t nargs = xargs.size();
		size_t set_idx = 0;
		for (size_t i=0; i<nargs; i++)
		{
			if (SET_LINK == xargs[i]->get_type())
			{
				set_idx = i;
				break;
			}
		}

		// Next, get the SetLink arity, and loop over it.
		size_t num_elts = xargs[set_idx]->get_arity();
		HandleSeq results;
		for (size_t n=0; n<num_elts; n++)
		{
			HandleSeq yargs;
			for (size_t i=0; i<nargs; i++)
			{
				if (i != set_idx)
					yargs.push_back(xargs[i]);
				else
					yargs.push_back(xargs[set_idx]->getOutgoingAtom(n));
			}
			results.push_back(vars.substitute_nocheck(body, yargs));
		}

		return createLink(std::move(results), SET_LINK);
	}

	return get_handle();
}

DEFINE_LINK_FACTORY(ExecutionOutputLink, EXECUTION_OUTPUT_LINK)
