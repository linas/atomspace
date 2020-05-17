/*
 * SexprEval.cc
 * Fast load of Atomese in s-expression format.
 *
 * Copyright (C) 2020 Alexey Potapov, Anatoly Belikov
 *
 * Authors: Alexey Potapov
 *          Anatoly Belikov
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
#include <vector>

#include <opencog/atoms/atom_types/NameServer.h>
#include <opencog/atoms/base/Link.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atomspace/AtomSpace.h>

#include "SexprEval.h"

using namespace opencog;

// Extract s-expression. Given a string `s`, update the `l` and `r`
// values so that `l` points at the next open-parenthsis (left paren)
// and `r` points at the matching close-paren.
static void get_next_expr(const std::string& s, size_t& l, size_t& r)
{
	while (s[l] != '(' and l < r) l++;
	if (l >= r) return;

	size_t p = l;
	int count = 1;
	bool quoted = false;
	do {
		p++;
		if (s[p] == '"')
		{
			if (0 < p and s[p - 1] != '\\')
				quoted = !quoted;
		}
		if (quoted) continue;
		if (s[p] == '(') count++;
		else if (s[p] == ')') count--;
	} while (p < r and count > 0);

	if (count != 0)
		throw std::runtime_error("Parenthsis mismatch");

	r = p;
}

// Tokenizer - extracts link or node type or name. Given the string `s`,
// this updates the `l` and `r` values such that `l` points at the first
// non-whitespace character of the name, and `r` points at the last.
// The string is considered to start *after* the first quote, and ends
// just before the last quote. In this case, escaped quotes \" are
// ignored (are considered to be part of the string).
static void get_next_token(const std::string& s, size_t& l, size_t& r)
{
	if (s[l] == '(') l++;

	// Advance past whitespace.
	for (; l < r and (s[l] == ' ' || s[l] == '\t' || s[l] == '\n'); l++);

	// We are parsing string
	if (s[l] == '"')
	{
		l++;
		size_t p = l;
		for (; p < r and (s[p] != '"' or ((0 < p) and (s[p - 1] == '\\'))); p++);
		r = p-1;
	}
	else
	{
		// Atom type name. Advance until whitespace.
		// Faster to use strtok!?
		size_t p = l;
		for (; l < r and s[p] != '(' and s[p] != ' ' and s[p] != '\t' and s[p] != '\n'; p++);
		r = p - 1;
	}
}

static NameServer& namer = nameserver();

// Parse the string `s`, returning a Handle that corresponds to that
// string.
static Handle recursive_parse(const std::string& s, size_t l, size_t r)
{
	size_t l1 = l, r1 = r;
	get_next_token(s, l1, r1);
	const std::string stype = s.substr(l1, r1);

	opencog::Type atype = namer.getType(stype);
	if (atype == opencog::NOTYPE)
	   throw std::runtime_error("Not an Atom");

	l = r1 + 1;
	if (namer.isLink(atype))
	{
		HandleSeq outgoing;
		do {
			l1 = l;
			r1 = r;
			get_next_expr(s, l1, r1);

			if (l1 == r1)
				throw std::runtime_error("Expecting Atom");

			outgoing.push_back(recursive_parse(s, l1, r1));

			l = r1 + 1;
		} while (l < r);

		return createLink(std::move(outgoing), atype);
	}
	else
	if (namer.isNode(atype))
	{
		l1 = l;
		r1 = r;
		get_next_token(s, l1, r1);
		if (l1 >= r1)
			throw std::runtime_error("Bad Atom name");

		size_t l2 = r1 + 1;
		size_t r2 = r;
		get_next_token(s, l2, r2);
		if (l2 < r2)
			throw std::runtime_error("Unexpexted stuff");

		const std::string name = s.substr(l1, r1);
		return createNode(atype, std::move(name));
	}
	throw std::runtime_error("Got a Value");
}

/// load_file -- load the given file into the given AtomSpace.
HandleSeq opencog::quick_eval(const std::string& expr)
{
	size_t len = expr.length();
	size_t l = 0;
	size_t r = len;

	HandleSeq hseq;
	while(l < r)
	{
		get_next_expr(expr, l, r);
		r++;
		Handle h = recursive_parse(expr, l, r);
		hseq.emplace_back(h);

		l = r + 1;
		r = len;
	}

	return hseq;
}
