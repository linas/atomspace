;
; filter-glob-test.scm -- Verify that globs in filters work.
;

(use-modules (opencog) (opencog exec))
(use-modules (opencog test-runner))

(opencog-test-runner)
(define tname "filter-glob-test")
(test-begin tname)

(cog-set-value! (Anchor "test") (Predicate "no-middle")
	(LinkValue
		(LinkValue
			(Concept "A")
			(Concept "B"))))

(cog-set-value! (Anchor "test") (Predicate "have-middle")
	(LinkValue
		(LinkValue
			(Concept "A")
			(Concept "MID")
			(Concept "B"))))

;; Test harness
(define (harness DECL MATCH PNAME)
	(cog-execute!
		(Filter
			(Rule
				(VariableList
					(Variable "$first")
					DECL
					(Variable "$last"))

				(LinkSignature (Type 'LinkValue)
					(Variable "$first")
					MATCH
					(Variable "$last"))
			(List (Variable "$first") (Variable "$last")))
			(ValueOf (Anchor "test") (Predicate PNAME)))))

(define simple-pair
	(LinkValue (List (Concept "A") (Concept "B"))))

;; Trivial match.
(define trivial-match
	(harness '() '() "no-middle"))
(format #t "trivial-match ~A\n" trivial-match)

(test-assert "trivial-match"
	(equal? trivial-match simple-pair))

(test-end tname)

(opencog-test-end)
