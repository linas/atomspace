/*
 * RewriteLink.cc
 *
 * Copyright (C) 2017 Nil Geisweiller
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

#include <string>

#include <opencog/util/random.h>
#include <opencog/util/Logger.h>
#include <opencog/atoms/base/ClassServer.h>
#include <opencog/atoms/core/TypeNode.h>
#include <opencog/atomutils/TypeUtils.h>
#include <opencog/atomutils/FindUtils.h>

#include "LambdaLink.h"
#include "RewriteLink.h"

using namespace opencog;

void RewriteLink::init(void)
{
	Type t = get_type();
	if (not classserver().isA(t, REWRITE_LINK))
	{
		const std::string& tname = classserver().getTypeName(t);
		throw InvalidParamException(TRACE_INFO,
			"Expecting a RewriteLink, got %s", tname.c_str());
	}
}

RewriteLink::RewriteLink(const Handle& vars, const Handle& body)
	: ScopeLink(HandleSeq({vars, body}), REWRITE_LINK), _silent(false)
{
	init();
}

RewriteLink::RewriteLink(const HandleSeq& oset, Type t)
	: ScopeLink(oset, t), _silent(false)
{
	if (skip_init(t)) return;
	init();
}

RewriteLink::RewriteLink(const Link &l)
	: ScopeLink(l), _silent(false)
{
	if (skip_init(l.get_type())) return;
	init();
}

/* ================================================================= */

inline Handle append_rand_str(const Handle& var)
{
	std::string new_var_name = randstr(var->get_name() + "-");
	return createNode(VARIABLE_NODE, new_var_name);
}

inline HandleSeq append_rand_str(const HandleSeq& vars)
{
	HandleSeq new_vars;
	for (const Handle& h : vars)
		new_vars.push_back(append_rand_str(h));
	return new_vars;
}

Handle RewriteLink::alpha_convert() const
{
	HandleSeq vars = append_rand_str(_varlist.varseq);
	return alpha_convert(vars);
}

Handle RewriteLink::alpha_convert(const HandleSeq& vars) const
{
	// Perform alpha conversion
	HandleSeq hs;
	for (size_t i = 0; i < get_arity(); ++i)
		hs.push_back(_varlist.substitute_nocheck(getOutgoingAtom(i), vars, _silent));

	// Create the alpha converted scope link
	return createLink(hs, get_type());
}

Handle RewriteLink::alpha_convert(const HandleMap& vsmap) const
{
	HandleSeq vars;
	for (const Handle& var : _varlist.varseq) {
		auto it = vsmap.find(var);
		vars.push_back(it == vsmap.end() ? append_rand_str(var) : it->second);
	}
	return alpha_convert(vars);
}

/* ================================================================= */

Handle RewriteLink::beta_reduce(const HandleMap& vm) const
{
	// Perform substitution over the variable declaration
	Handle nvardecl = substitute_vardecl(vm);

	// Perform substitution over the bodies
	HandleSeq hs = substitute_bodies(nvardecl, vm);

	// Filter vardecl
	nvardecl = filter_vardecl(nvardecl, hs);

	// Insert vardecl in the outgoing set, if defined
	if (nvardecl)
	{
		hs.insert(hs.begin(), nvardecl);
	}
	else
	{
		// Its illegal to create a PutLink that is not in the
		// form of a redex, i.e. doesn't have variable declarations
		// in it. So we must not call createLink(), below.
		Type t = get_type();
		if (PUT_LINK == t or LAMBDA_LINK == t)
			return hs[0];
	}

	// Create the substituted scope.  I suspect that this is a bad
	// idea, when nvardecl==nullptr, I mean, its just gonna be weird,
	// and cause issues thhrought the code ... but ... whatever.
	return createLink(hs, get_type());
}

Handle RewriteLink::beta_reduce(const HandleSeq& vals) const
{
	const Variables& vars = get_variables();
	if (vals.size() != vars.size())
	{
		if (1 != vals.size() or LAMBDA_LINK != vals[0]->get_type())
		{
			if (_silent) throw TypeCheckException();

			throw SyntaxException(TRACE_INFO,
				"RewriteLink has mismatched arity, expecting %lu == %lu",
				vars.size(), vals.size());
		}

		// Verify that eta reduction is possible...
		LambdaLinkPtr lam(LambdaLinkCast(vals[0]));
		const Handle& body = lam->get_body();

		if (body->get_arity() != vars.size() or
		    body->get_type() != LIST_LINK)
		{
			if (_silent) throw TypeCheckException();

			throw SyntaxException(TRACE_INFO,
				"RewriteLink has mismatched eta, expecting %lu == %lu",
				vars.size(), body->get_arity());
		}

		// Perform a very simple-minded eta reduction.
		// This will be incorrect under a variety of corner cases,
		// but gives approximately correct results for the simple case.
		const HandleSeq& eta = body->getOutgoingSet();
		HandleMap vm;

		for (size_t i=0; i<eta.size(); i++)
			vm.insert({vars.varseq[i], eta[i]});
		return beta_reduce(vm);
	}

	HandleMap vm;
	for (size_t i=0; i<vals.size(); i++)
	{
		vm.insert({vars.varseq[i], vals[i]});
	}
	return beta_reduce(vm);
}

HandleSeq RewriteLink::substitute_bodies(const Handle& nvardecl,
                                         const HandleMap& vm) const
{
	const Variables& variables = get_variables();
	return beta_reduce_bodies(nvardecl, variables.make_sequence(vm));
}

HandleSeq RewriteLink::beta_reduce_bodies(const Handle& nvardecl,
                                         const HandleSeq& values) const
{
	HandleSeq hs;
	for (size_t i = (get_vardecl() ? 1 : 0); i < get_arity(); ++i)
	{
		const Handle& h = getOutgoingAtom(i);
		hs.push_back(substitute_body(nvardecl, h, values));
	}
	return hs;
}

Handle RewriteLink::substitute_body(const Handle& nvardecl,
                                    const Handle& body,
                                    const HandleSeq& values) const
{
	Handle nbody = get_variables().substitute(body, values, _silent);
	nbody = consume_ill_quotations(nvardecl, nbody);
	return nbody;
}

Handle RewriteLink::substitute_vardecl(const HandleMap& vm) const
{
	if (not get_vardecl())
		return Handle::UNDEFINED;

	return substitute_vardecl(get_vardecl(), vm);
}

Handle RewriteLink::substitute_vardecl(const Handle& vardecl,
                                       const HandleMap& vm)
{
	Type t = vardecl->get_type();

	// Base cases

	if (t == VARIABLE_NODE)
	{
		auto it = vm.find(vardecl);

		// Only substitute if the variable is substituted by another variable
		if (it == vm.end())
			return vardecl;
		if (it->second->get_type() == VARIABLE_NODE)
			return it->second;
		return Handle::UNDEFINED;
	}

	// Recursive cases

	HandleSeq oset;

	if (t == VARIABLE_LIST)
	{
		for (const Handle& h : vardecl->getOutgoingSet())
		{
			Handle nh = substitute_vardecl(h, vm);
			if (nh)
				oset.push_back(nh);
		}
		if (oset.empty())
			return Handle::UNDEFINED;
	}
	else if (t == TYPED_VARIABLE_LINK)
	{
		Handle new_var = substitute_vardecl(vardecl->getOutgoingAtom(0), vm);
		if (new_var)
		{
			oset.push_back(new_var);
			oset.push_back(vardecl->getOutgoingAtom(1));
		}
		else
			return Handle::UNDEFINED;
	}
	else
	{
		OC_ASSERT(false, "Not implemented");
	}
	return createLink(oset, t);
}

Handle RewriteLink::consume_ill_quotations() const
{
	Handle vardecl = get_vardecl();
	const Variables& variables = get_variables();
	HandleSeq nouts;
	for (size_t i = (get_vardecl() ? 1 : 0); i < get_arity(); ++i)
	{
		Handle nbody = consume_ill_quotations(variables, getOutgoingAtom(i));
		nouts.push_back(nbody);
		// If the new body has terms with free variables but no
		// vardecl it means that some quotations are missing. Rather
		// than adding them we set vardecl to an empty VariableList.
		if (not vardecl and not get_free_variables(nbody).empty())
			vardecl = Handle(createVariableList(HandleSeq{}));
	}

	if (vardecl)
		nouts.insert(nouts.begin(), vardecl);

	// Recreate the scope
	return createLink(nouts, get_type());
}

Handle RewriteLink::consume_ill_quotations(const Handle& vardecl,
                                           const Handle& h)
{
	return consume_ill_quotations(gen_variables(h, vardecl), h);
}

Handle RewriteLink::consume_ill_quotations(const Variables& variables, Handle h,
                                           Quotation quotation, bool escape)
{
	// Base case
	if (h->is_node())
		return h;

	// Recursive cases
	Type t = h->get_type();
	if (quotation.consumable(t)) {
		if (t == QUOTE_LINK) {
			Handle qh = h->getOutgoingAtom(0);
			// If it's a scope, check whether its vardecl is bound to
			// itself rather than the ancestor scope, if so the Quote
			// is harmful, consume it. Otherwise, for other quoted
			// link types, do not consume the quote and the subsequent
			// unquotes.
			if (classserver().isA(qh->get_type(), SCOPE_LINK) and
			    not is_bound_to_ancestor(variables, qh))
			{
				quotation.update(t);
				return consume_ill_quotations(variables, qh, quotation);
			} else {
				escape = true;
			}
		} else if (t == UNQUOTE_LINK) {
			Handle uh = h->getOutgoingAtom(0);
			// Either remove subsequent unquote associated by a
			// removed quote, or useless unquote because there are no
			// free variables to unquote
			if (not escape or get_free_variables(uh).empty()) {
				quotation.update(t);
				return consume_ill_quotations(variables, h->getOutgoingAtom(0),
				                              quotation);
			}
		}
		// Ignore LocalQuotes as they supposedly used only to quote
		// pattern matcher connectors.
	}

	quotation.update(t);
	HandleSeq consumed;
	for (const Handle outh : h->getOutgoingSet())
		consumed.push_back(consume_ill_quotations(variables, outh, quotation,
		                                          escape));

	return createLink(consumed, t);
}

bool RewriteLink::is_bound_to_ancestor(const Variables& variables,
                                       const Handle& local_scope)
{
	Handle unquote = local_scope->getOutgoingAtom(0);
	if (unquote->get_type() == UNQUOTE_LINK) {
		Handle var = unquote->getOutgoingAtom(0);
		return variables.is_in_varset(var);
	}
	return false;
}

/* ================================================================= */

DEFINE_LINK_FACTORY(RewriteLink, REWRITE_LINK);

/* ===================== END OF FILE ===================== */
