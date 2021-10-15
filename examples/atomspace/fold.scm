
(use-modules (opencog) (opencog exec))

; TODO maybe a ForEachLink to apply a function to a list or a set?
(define a-set
	(Set
		(Concept "A" (CountTruthValue 1 0 1))
		(Concept "B" (CountTruthValue 1 0 2))
		(Concept "C" (CountTruthValue 1 0 3))))

(cog-prt-atomspace)

; (cog-execute! (CountOf a-set))
; (cog-execute! (CountOf (ForEach a-set)))
; Perhaps ForEach returns a LinkValue that CountValue can process??

(define qry
	(Meet
		(TypedVariable (Variable "$c") (Type 'Concept))
		(Present (Variable "$c"))))

(cog-execute! qry)

; Ideas:
; (cog-execute! (CountOf qry))
; (cog-execute! (CountOf (Run qry)))
;

; Desired result:
(define sum
	(Fold
		(Lamda
			(VariableList (Variable "$itm") (Variable "$acc"))
			(Plus (CountOf (Variable "$item")) (Variable "$acc")))
		(Number 0)
		qry))
; (cog-execute! sum)  ; return 6 (a umber, or a FloatValue?)

; Change NumberNode to be executable, and when it executes,
; it runs it's argument. If the arg returns a FloatValue, the
; the NumberNode stores that.


; ----------------------
; Fancier version -- the dot product

; A pair of vectors
(Evaluation (Predicate "has legs") (Concept "dog") (CountTruthValue 1 0 1))
(Evaluation (Predicate "has nose") (Concept "dog") (CountTruthValue 1 0 2))
(Evaluation (Predicate "has tail") (Concept "dog") (CountTruthValue 1 0 3))
(Evaluation (Predicate "furry")    (Concept "dog") (CountTruthValue 1 0 4))
(Evaluation (Predicate "domestic") (Concept "dog") (CountTruthValue 1 0 5))

(Evaluation (Predicate "has legs") (Concept "cat") (CountTruthValue 1 0 1))
(Evaluation (Predicate "has nose") (Concept "cat") (CountTruthValue 1 0 2))
(Evaluation (Predicate "has tail") (Concept "cat") (CountTruthValue 1 0 3))
(Evaluation (Predicate "furry")    (Concept "cat") (CountTruthValue 1 0 4))
(Evaluation (Predicate "domestic") (Concept "cat") (CountTruthValue 1 0 5))

; A search that returns the left-prpduct
(define qdot-classic
	(Query
		(TypedVariable (Variable "$prop") (Type 'Predicate))
		(Present
			(Evaluation (Variable "$prop") (Concept "dog"))
			(Evaluation (Variable "$prop") (Concept "cat")))
		(List
			(Evaluation (Variable "$prop") (Concept "dog"))
			(Evaluation (Variable "$prop") (Concept "cat")))))

; Wow -- this actually works!
(define qdot-math
	(Query
		(TypedVariable (Variable "$prop") (Type 'Predicate))
		(Present
			(Evaluation (Variable "$prop") (Concept "dog"))
			(Evaluation (Variable "$prop") (Concept "cat")))
		(Times
			(CountOf (Evaluation (Variable "$prop") (Concept "dog")))
			(CountOf (Evaluation (Variable "$prop") (Concept "cat"))))))

(define dot
	(Fold
		(Lamda
			(VariableList (Variable "$itm") (Variable "$acc"))
			(Plus
				(Times
					(CountOf
				(Variable "$item")) (Variable "$acc")))
		(Plus (Times (CountOf (Variable "$X")))
		(Number 0)
		qdot))

