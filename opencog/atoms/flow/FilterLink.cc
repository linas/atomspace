/*
 * opencog/atoms/flow/FilterLink.cc
 *
 * Copyright (C) 2015, 2016, 2022 Linas Vepstas
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
#include <opencog/atoms/base/ClassServer.h>
#include <opencog/atoms/core/DefineLink.h>
#include <opencog/atoms/core/FindUtils.h>
#include <opencog/atoms/core/TypeNode.h>
#include <opencog/atoms/core/TypeUtils.h>
#include <opencog/atoms/core/VariableSet.h>
#include <opencog/atoms/rule/RuleLink.h>
#include <opencog/atoms/value/LinkValue.h>
#include <opencog/atoms/value/ContainerValue.h>

#include "FilterLink.h"
#include "LinkSignatureLink.h"
#include "ValueShimLink.h"

using namespace opencog;

void FilterLink::init(void)
{
	// Filters consist of a function, and the data to apply the
	// function to.  The function can be explicit (inheriting from
	// ScopeLink) or implicit (we automatically fish out free variables).
	if (2 != _outgoing.size())
		throw SyntaxException(TRACE_INFO,
			"FilterLink is expected to be arity-2 only!");

	Handle termpat = _outgoing[0];
	Type tscope = termpat->get_type();

	// Expand definitions
	if (nameserver().isA(tscope, DEFINED_PROCEDURE_NODE))
	{
		termpat = DefineLink::get_definition(termpat);
		if (nullptr == termpat)
			throw SyntaxException(TRACE_INFO,
				"FilterLink cannot find definition for %s",
				_outgoing[0]->to_string().c_str());

		tscope = termpat->get_type();
	}

	// First argument must be a function of some kind.  All functions
	// are specified using a ScopeLink, to bind the input-variables.
	if (nameserver().isA(tscope, SCOPE_LINK))
	{
		_pattern = ScopeLinkCast(termpat);
	}
	else
	{
		const Handle& body = termpat;
		FreeVariables fv;
		fv.find_variables(body);
		Handle decl(createVariableSet(std::move(fv.varseq)));
		_pattern = createScopeLink(std::move(decl), body);
	}
	_mvars = &_pattern->get_variables();
	_varset = &_mvars->varset;

	// RuleLinks are a special type of ScopeLink.  They specify a
	// re-write that should be performed.  Viz, RuleLinks are
	// of the form P(x)->Q(x).  Here, the `_rewrite` is the Q(x)
	if (nameserver().isA(tscope, RULE_LINK))
		_rewrite = RuleLinkCast(HandleCast(_pattern))->get_implicand();

	// Locate all GlobNodes in the pattern
	FindAtoms fgn(GLOB_NODE, true);
	fgn.search_set(_pattern->get_body());
	for (const Handle& sh : fgn.least_holders)
		_globby_terms.insert(sh);

	// FunctionLink::init() will extract free variables. But we don't
	// really need this for anything, so don't run it.
	// FunctionLink::init();
}

FilterLink::FilterLink(const Handle& pattern, const Handle& term)
	: FunctionLink(HandleSeq({pattern, term}), FILTER_LINK)
{
	init();
}

FilterLink::FilterLink(Type t, const Handle& body)
	: FunctionLink(HandleSeq({body}), t)
{
	// Derived types have a different initialization sequence.
	if (FILTER_LINK != t) return;
	init();
}

FilterLink::FilterLink(const HandleSeq&& oset, Type t)
	: FunctionLink(std::move(oset), t)
{
	if (not nameserver().isA(t, FILTER_LINK))
	{
		const std::string& tname = nameserver().getTypeName(t);
		throw SyntaxException(TRACE_INFO,
			"Expecting a FilterLink, got %s", tname.c_str());
	}

	// Derived types have a different initialization sequence.
	if (FILTER_LINK != t) return;
	init();
}

// ===============================================================

/// Recursive tree-compare-and-extract grounding values.
///
/// Compare the pattern tree `termpat` with the grounding tree `ground`.
/// If a variable in `termpat` corresponds with a variable in `ground`,
/// then add that correspondence pair to `valmap`. Type-checking is
/// performed during the match-up, so if the variable type does not
/// match the ground type, false is returned.  False is also returned
/// if the trees miscompare in other ways (mismatched link arity,
/// mis-matched atom type, two conflicting groundings for the same
/// variable).
///
/// Any executable terms in `ground` are executed prior to comparison.
/// XXX Is executing the ground a good design choice? I dunno. It's the
/// historical choice. So it goes.
///
/// If false is returned, the contents of valmap are invalid. If true
/// is returned, valmap contains the extracted values.
///
bool FilterLink::extract(const Handle& termpat,
                         const ValuePtr& gnd,
                         ValueMap& valmap,
                         AtomSpace* scratch, bool silent,
                         Quotation quotation) const
{
	if (termpat == gnd) return true;

	ValuePtr vgnd(gnd);

	// Execute the proposed grounding term, first. Notice that this is
	// a "deep" execution, because there may have been lots of
	// non-executable stuff above us. Is this deep execution actually
	// a good idea? I dunno; this is an older design decision, motivated
	// by the URE. Is it a good design decision? I dunno. For now, there's
	// not enough experience to say. There is, however, a unit test to
	// check this behavior.
	Handle hgnd(HandleCast(gnd));
	if (hgnd and hgnd->is_executable())
	{
		vgnd = hgnd->execute(scratch, silent);
		if (nullptr == vgnd) return false;
	}

	// Let the conventional type-checker deal with complicated types.
	// LinkSignatureLinks might contain vars; deal with these below.
	if (termpat->is_type(TYPE_NODE) or
	    (termpat->is_type(TYPE_OUTPUT_LINK) and
	       (not termpat->is_type(LINK_SIGNATURE_LINK))))
		return value_is_type(termpat, vgnd);

	Type t = termpat->get_type();
	// If its a variable, then see if we know its value already;
	// If not, then record it.
	if (VARIABLE_NODE == t and 0 < _varset->count(termpat))
	{
		auto val = valmap.find(termpat);
		if (valmap.end() != val)
		{
			// If we already have a value, the value must be identical.
			return (val->second == vgnd);
		}

		// Check the type of the value.
		if (not _mvars->is_type(termpat, vgnd)) return false;

		// If we are here, everything looks good. Record and return.
		valmap.emplace(std::make_pair(termpat, vgnd));
		return true;
	}

	// Save quotation state before updating it
	Quotation quotation_cp;
	quotation.update(t);

	// Consume quotation
	if (quotation_cp.consumable(t))
		return extract(termpat->getOutgoingAtom(0), vgnd, valmap,
		               scratch, silent, quotation);

	if (GLOB_NODE == t and 0 < _varset->count(termpat))
	{
		// Check the type of the value.
		if (not _mvars->is_type(termpat, vgnd)) return false;

		// If we are deep in doing glob compares, then all that was
		// needed is the result of the the type compare. However,
		// if this is a top-level glob that is wrapping everything,
		// then we need to record it (immediately below).
		if (_recursive_glob) return true;

		// Globs are always wrapped, no matter what, by a List or
		// a LinkValue, because for the general case, the arity
		// is unknown (even though its exactly one, here.)
		if (vgnd->is_atom())
			valmap.emplace(std::make_pair(termpat,
			               createLink(LIST_LINK, HandleCast(vgnd))));
		else
			valmap.emplace(std::make_pair(termpat, createLinkValue(vgnd)));
		return true;
	}

	// Special-case for ChoiceLinks in the body of the pattern.
	// This dangles one foot over the edge of a slippery slope,
	// of analyzing the body of the map and special-casing. Not
	// sure if this is a good idea, or a bad idea...
	if (CHOICE_LINK == t)
	{
		for (const Handle& choice : termpat->getOutgoingSet())
		{
			// Push and pop valmap each go-around; some choices
			// might partly work, and corrupt the map.
			ValueMap vcopy(valmap);
			if (extract(choice, vgnd, vcopy, scratch, silent, quotation))
			{
				valmap.swap(vcopy);
				return true;
			}
		}
		return false;
	}

	// Search for a specific StringValue. There's not way to
	// place StringValues directly into the pattern, but it
	// can be encoded with StringOfLink. Note this is a content
	// compare, not a pointer compare, because values are not
	// de-duped.
	if (STRING_OF_LINK == t)
	{
		ValuePtr patval = termpat->execute();
		return (*patval == *vgnd);
	}

	// If they are (non-variable) nodes, they must be identical.
	if (not termpat->is_link())
		return (termpat == vgnd);

	// Pattern is a LinkSig. LinkSigs have a typespec. Does the type
	// of the proposed grounding agree with the LinkSig typespec?
	// If not, bounce out.
	// The LinkSig typespec might be TypeNode, TypeInhNode or TypeCoInhNode
	// The TypeNode::is_kind() handles all three cases.
	if (LINK_SIGNATURE_LINK == t)
	{
		// Type of LinkSig is encoded in the first atom.
		Handle ts = LinkSignatureLinkCast(termpat)->get_typespec();
		if (not TypeNodeCast(ts)->is_kind(vgnd->get_type()))
			return false;
	}

	// Else straight-up see if pattern and grounding types agree.
	else if (t != vgnd->get_type()) return false;

	// From here on out, we prepare to compare Links.
	const HandleSeq& tlo = termpat->getOutgoingSet();
	size_t tsz = tlo.size();
	size_t off = 0;
	if (LINK_SIGNATURE_LINK == t)
		{ tsz--; off = 1; }

	// If no glob nodes, just compare links side-by-side.
	if (0 == _globby_terms.count(termpat))
	{
		// This and the next block are nearly identical, except that in
		// the first, glo is HandleSeq while the second  is ValueSeq.
		// We could handle this with a template, but the blocks are so
		// short, that the template boilerplate is longer than the block.
		if (vgnd->is_atom())
		{
			const HandleSeq& glo = HandleCast(vgnd)->getOutgoingSet();
			size_t gsz = glo.size();
			// If the sizes are mismatched, should we do a fuzzy match?
			if (tsz != gsz) return false;
			for (size_t i=0; i<tsz; i++)
			{
				if (not extract(tlo[i+off], glo[i], valmap, scratch, silent, quotation))
					return false;
			}
			return true;
		}

		if (vgnd->is_type(LINK_VALUE))
		{
			// Loop just like above.
			const auto& glo = LinkValueCast(vgnd)->value();
			size_t gsz = glo.size();
			// If the sizes are mismatched, should we do a fuzzy match?
			if (tsz != gsz) return false;
			for (size_t i=0; i<tsz; i++)
			{
				if (not extract(tlo[i+off], glo[i], valmap, scratch, silent, quotation))
					return false;
			}
			return true;
		}

		// If we are here, we have a solitary Value. We already know
		// that it has the right type; else we would not have gotten
		// here. Do some paranoid checks, anyway.
		if ((LINK_SIGNATURE_LINK != t) or (1 != tsz))
			return false;

		valmap.emplace(std::make_pair(tlo[1], vgnd));
		return true;
	}

	// If we are here, then there's a glob to be matched. As just above,
	// the HandleSeq and ValueSeq variants are effectively identical.
	_recursive_glob = true;

	// Helper lambda to perform glob matching for a given ground sequence type
	auto do_glob_match = [&]<typename GroundSeq>(const GroundSeq& glo) -> bool
	{
		// Validation callback - delegates to extract()
		GlobValidateCallback<GroundSeq> validate =
			[&](const Handle& pattern_elem,
			    const typename GroundSeq::value_type& ground_elem,
			    ValueMap& bindings) -> bool {
				return this->extract(pattern_elem, ground_elem, bindings,
				                     scratch, silent, quotation);
			};

		// Value creation callback - creates appropriate container type
		GlobMakeValueCallback<GroundSeq> make_value;
		if constexpr (std::is_same_v<GroundSeq, HandleSeq>)
			make_value = [](const HandleSeq& matched_seq) -> ValuePtr {
				return createLink(HandleSeq(matched_seq), LIST_LINK);
			};
		else
			make_value = [](const ValueSeq& matched_seq) -> ValuePtr {
				return createLinkValue(ValueSeq(matched_seq));
			};

		return glob_match(tlo, glo, valmap, _mvars, validate, make_value, off, tsz);
	};

	// Handle different ground value types
	if (vgnd->is_link())
		return do_glob_match(HandleCast(vgnd)->getOutgoingSet());

	if (vgnd->is_type(LINK_VALUE))
		return do_glob_match(LinkValueCast(vgnd)->value());

	// Single value case
	return do_glob_match(ValueSeq({vgnd}));
}

// ====================================================================

ValuePtr FilterLink::rewrite_one(const ValuePtr& vterm,
                                 AtomSpace* scratch, bool silent) const
{
	// temp hack
	Handle cterm(HandleCast(vterm));

	// See if the term passes pattern matching. If it does, the
	// side effect is that we get a grounding map as output.
	ValueMap valmap;
	_recursive_glob = false;
	if (not extract(_pattern->get_body(), vterm, valmap, scratch, silent))
		return Handle::UNDEFINED;

	// Special case for signatures. The extract already rejected
	// mis-matches, if any. Thus, we are done, here.
	const Handle& body(_pattern->get_body());
	if (body->is_type(TYPE_NODE) or
	    (body->is_type(TYPE_OUTPUT_LINK) and
	       (not body->is_type(LINK_SIGNATURE_LINK))))
		return vterm;

	// If there is no RuleLink to fire, then just wrap up the results
	// and return them.
	if (_rewrite.empty())
	{
		if (1 == valmap.size()) return valmap.begin()->second;

		// Multiple Values to return. Two generic cases: the return
		// value is a set of Atoms, or a set of non-Atom Values.
		if (body->is_type(LINK_SIGNATURE_LINK))
		{
			ValueSeq valseq;
			for (const Handle& var : _mvars->varseq)
			{
				const auto& valpair = valmap.find(var);
				valseq.emplace_back(valpair->second);
			}
			return LinkSignatureLinkCast(body)->construct(std::move(valseq));
		}

		// A list of Handles.
		HandleSeq valseq;
		for (const Handle& var : _mvars->varseq)
		{
			const auto& valpair = valmap.find(var);
			valseq.emplace_back(HandleCast(valpair->second));
		}
		return scratch->add_link(LIST_LINK, std::move(valseq));
	}

	// If we are there, then there's a rule to fire. Two generic
	// cases to be handled:
	// 1) If the rewrite rule is not executable, then the grounding
	//    must consist of Atoms only. The grounding is plugged into
	//    the rewrite, i.e. beta-reduced.
	// 2) If the rewrite rule is executable, then the grounding
	//    is handed to that, without any further ado. The grounding
	//    might be either Values or Atoms; in either case, they're
	//    arguments for the function to be executed.

	// Place the groundings into a sequence, for easy access.
	HandleSeq valseq;
	for (const Handle& var : _mvars->varseq)
	{
		auto valpair = valmap.find(var);

		if (valmap.end() == valpair)
			throw FatalErrorException(TRACE_INFO,
				"Internal error; bug in filtering code");

		const ValuePtr& gnding(valpair->second);

		if (gnding->is_atom())
			valseq.emplace_back(HandleCast(gnding));
		else
		{
			ValueShimLinkPtr wrap(createValueShimLink());
			wrap->set_value(gnding);
			valseq.emplace_back(wrap);
		}
	}

	ValueSeq rew;
	// Beta reduce, and execute. No type-checking during
	// beta-reduction; we've already done that, during matching.
	for (const Handle& impl : _rewrite)
	{
		ValuePtr red(_mvars->substitute_nocheck(impl, valseq, false, true));
		if (red->is_atom())
		{
			const Handle& hred(HandleCast(red));
			if (hred->is_executable())
			{
				ValuePtr v(hred->execute(scratch, silent));
				if (v) rew.emplace_back(v);
			}
			else
				rew.emplace_back(scratch->add_atom(hred));
		}
		else
			rew.emplace_back(red);
	}

	if (1 == rew.size()) return rew[0];

	bool have_vals = false;
	for (const ValuePtr& v : rew)
		if (v and not v->is_atom()) { have_vals = true; break; }

	// Multiple Values to return. Two generic cases: the return
	// value is a set of Atoms, or a set of non-Atom Values.
	if (have_vals or body->is_type(LINK_SIGNATURE_LINK))
	{
		// Type kind = LinkSignatureLinkCast(body)->get_kind();
		// if (LINK_VALUE == kind) ...
		return createLinkValue(std::move(rew));
	}

	// A list of Handles.
	HandleSeq hseq;
	for (const ValuePtr& v : rew) hseq.emplace_back(HandleCast(v));
	return scratch->add_link(LIST_LINK, std::move(hseq));
}

ValuePtr FilterLink::execute(AtomSpace* as, bool silent)
{
	ValuePtr vex(_outgoing[1]);

	if (_outgoing[1]->is_executable())
	{
		vex = _outgoing[1]->execute(as, silent);

		// If it's a container, and its not closed, then pull
		// one value out, and process it. Else if its closed,
		// fall through, and let the next stage handle it.
		if (vex->is_type(CONTAINER_VALUE))
		{
			ContainerValuePtr cvp = ContainerValueCast(vex);
			if (not cvp->is_closed())
			{
				ValuePtr mone = rewrite_one(cvp->remove(), as, silent);
				return createLinkValue(mone);
			}
			// If it is closed, fall through.
		}

		if (vex->is_type(LINK_VALUE))
		{
			ValueSeq remap;
			for (const ValuePtr& vp : LinkValueCast(vex)->value())
			{
				ValuePtr mone = rewrite_one(vp, as, silent);
				if (nullptr != mone) remap.emplace_back(mone);
			}
			return createLinkValue(std::move(remap));
		}
	}

	// Handle four different cases.
	// If there is a single Atom, apply the filter to the single Atom.
	// If there is a Set of Atoms, apply the filter to the set members.
	// If there is a List of Atoms, apply the filter to the list elts.
	// If there is a LinkValue, apply the filter to its elts.
	Type argtype = vex->get_type();
	if (SET_LINK == argtype or LIST_LINK == argtype)
	{
		Handle valh(HandleCast(vex));
		HandleSeq remap;
		for (const Handle& h : valh->getOutgoingSet())
		{
			Handle mone = HandleCast(rewrite_one(h, as, silent));
			if (nullptr != mone) remap.emplace_back(mone);
		}
		return as->add_link(argtype, std::move(remap));
	}

	// If we're getting a shim, we MUST unwrap it. If the shim
	// wraps a LinkValue, iterate over that.
	if (VALUE_SHIM_LINK == argtype)
	{
		Handle valh(HandleCast(vex));
		ValuePtr svp(valh->execute(as, silent));
		if (LINK_VALUE != svp->get_type()) return svp;

		ValueSeq remap;
		const ValueSeq& vsq(LinkValueCast(svp)->value());
		for (const ValuePtr& v: vsq)
			remap.emplace_back(rewrite_one(v, as, silent));

		if (1 == remap.size()) return remap[0];
		return createLinkValue(std::move(remap));
	}

	// Its a singleton. Just remap that.
	return rewrite_one(vex, as, silent);
}

DEFINE_LINK_FACTORY(FilterLink, FILTER_LINK)

/* ===================== END OF FILE ===================== */
