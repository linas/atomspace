;
; always-mono-test.scm -- Test MonoValue as a replacement for AlwaysLink
;
; Copyright (C) 2019 Linas Vepstas
; All Rights Reserved
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU Affero General Public License v3 as
; published by the Free Software Foundation and including the exceptions
; at http://opencog.org/wiki/Licenses
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU Affero General Public License
; along with this program; if not, write to:
; Free Software Foundation, Inc.,
; 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
;
; This is a port of AlwaysUTest::test_basic() to use MonoValue instead
; of AlwaysLink.
;

(use-modules (opencog))
(use-modules (opencog exec))
(use-modules (opencog test-runner))

(opencog-test-runner)

; ---------------------------------------------------------------
; Data from always.scm (inlined)

; Three baskets holding balls
(Inheritance (Concept "reds basket") (Concept "basket"))
(Inheritance (Concept "reds&greens basket") (Concept "basket"))
(Inheritance (Concept "yellows basket") (Concept "basket"))

; Balls placed into baskets
(Member (Concept "red ball")      (Concept "reds basket"))
(Member (Concept "red ball too")  (Concept "reds basket"))
(Member (Concept "red ball also") (Concept "reds basket"))

(Member (Concept "red ball")     (Concept "reds&greens basket"))
(Member (Concept "red ball too") (Concept "reds&greens basket"))
(Member (Concept "green ball")   (Concept "reds&greens basket"))

(Member (Concept "yellow ball") (Concept "yellows basket"))
(Member (Concept "ochre ball")  (Concept "yellows basket"))

; Colors of the balls
(Evaluation (Predicate "is red") (Concept "red ball"))
(Evaluation (Predicate "is red") (Concept "red ball too"))
(Evaluation (Predicate "is red") (Concept "red ball also"))

(Evaluation (Predicate "is green")  (Concept "green ball"))

(Evaluation (Predicate "is yellow") (Concept "yellow ball"))
(Evaluation (Predicate "is yellow") (Concept "ochre ball"))

; ---------------------------------------------------------------
; Query from always.scm: baskets-with-red-balls-only
; Original used: (Always (Evaluation (Predicate "is red") (Variable "ball")))
; With MonoValue, we check each basket individually.

(define (basket-has-only-red-balls? basket)
	; Query to find balls in this basket that are red
	(define query
		(Query
			(TypedVariable (Variable "ball") (Type 'ConceptNode))
			(And
				(Member (Variable "ball") basket)
				(Evaluation (Predicate "is red") (Variable "ball")))
			(Variable "ball")))

	; MonoValue with trivially-true equivalence (all reds are equivalent)
	(define checker
		(MonoValue
			(Lambda
				(VariableList (Variable "$A") (Variable "$B"))
				(Identical (Variable "$A") (Variable "$A")))))

	(cog-set-value! query query checker)
	(cog-execute! query)

	; Query to count all balls in basket
	(define all-balls-query
		(Meet
			(TypedVariable (Variable "ball") (Type 'ConceptNode))
			(Member (Variable "ball") basket)))
	(define all-balls (cog-value->list (cog-execute! all-balls-query)))
	(define red-balls (cog-value->list checker))

	; Basket has only red balls if count matches and is non-zero
	(and (not (null? red-balls))
	     (= (length red-balls) (length all-balls))))

; ---------------------------------------------------------------
; Query from always.scm: baskets-with-same-color
; Original used: (Always (Equal (Variable "some color") (Variable "other color")))
; With MonoValue, we check if all colors in a basket are identical.

(define (basket-has-same-color? basket)
	; Query to find colors of balls in this basket
	(define query
		(Query
			(VariableList
				(TypedVariable (Variable "ball") (Type 'ConceptNode))
				(TypedVariable (Variable "color") (Type 'PredicateNode)))
			(And
				(Member (Variable "ball") basket)
				(Evaluation (Variable "color") (Variable "ball")))
			(Variable "color")))

	; MonoValue checks if all colors are identical
	(define checker
		(MonoValue
			(Lambda
				(VariableList (Variable "$A") (Variable "$B"))
				(Identical (Variable "$A") (Variable "$B")))))

	(cog-set-value! query query checker)
	(cog-execute! query)

	; If MonoValue is non-empty, all colors were the same
	(not (null? (cog-value->list checker))))

; ---------------------------------------------------------------
; Test baskets-with-red-balls-only
; Expecting: 1 basket (reds basket)
;
(define tname-red "baskets-with-red-balls-only")
(test-begin tname-red)

(define all-baskets
	(cog-value->list
		(cog-execute!
			(Meet
				(TypedVariable (Variable "basket") (Type 'ConceptNode))
				(Inheritance (Variable "basket") (Concept "basket"))))))

(define red-only-baskets
	(filter basket-has-only-red-balls? all-baskets))

(format #t "Expecting red basket, got ~A\n" red-only-baskets)

(test-assert "one-red-basket" (= 1 (length red-only-baskets)))
(test-assert "is-reds-basket"
	(member (Concept "reds basket") red-only-baskets))

(test-end tname-red)

; ---------------------------------------------------------------
; Test baskets-with-same-color
; Expecting: 2 baskets (reds basket, yellows basket)
;
(define tname-same "baskets-with-same-color")
(test-begin tname-same)

(define same-color-baskets
	(filter basket-has-same-color? all-baskets))

(format #t "Expecting red and yellow, got ~A\n" same-color-baskets)

(test-assert "two-same-color-baskets" (= 2 (length same-color-baskets)))
(test-assert "has-reds-basket"
	(member (Concept "reds basket") same-color-baskets))
(test-assert "has-yellows-basket"
	(member (Concept "yellows basket") same-color-baskets))

(test-end tname-same)

(opencog-test-end)
