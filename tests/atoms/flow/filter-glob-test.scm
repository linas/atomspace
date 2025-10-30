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

;-----------------
; Possible results from above harness.
(define simple-pair
	(LinkValue (List (Concept "A") (Concept "B"))))

(define empty (LinkValue))

;-----------------
;; Trivial match.
(format #t "================================== trivial-match\n")
(define trivial-match
	(harness '() '() "no-middle"))

(format #t "trivial-match ~A\n" trivial-match)
(test-assert "trivial-match" (equal? trivial-match simple-pair))

;-----------------
;; Trivial mis-match.
(format #t "================================== mis-triv-match\n")
(define mis-triv-match
	(harness '() '() "have-middle"))

(format #t "mis-triv-match ~A\n" mis-triv-match)
(test-assert "mis-triv-match" (equal? mis-triv-match empty))

;-----------------
;; Untyped Glob; requires one or more matches
(format #t "================================== mid-mis-match\n")
(define mid-mis-match
	(harness (Glob "$middle") (Glob "$middle") "no-middle"))

(format #t "mid-mis-match ~A\n" mid-mis-match)
(test-assert "mid-mis-match" (equal? mid-mis-match empty))

;-----------------
;; Untyped Glob; requires one or more matches
(format #t "================================== middle-match\n")
(define middle-match
	(harness (Glob "$middle") (Glob "$middle") "have-middle"))

(format #t "middle-match ~A\n" middle-match)
(test-assert "middle-match" (equal? middle-match simple-pair))

;-----------------
;; Typed Glob; requires zero or more matches
(format #t "================================== type-zero-match\n")
(define type-zero-match
	(harness
		(TypedVariable (Glob "$middle")
			(Interval (Number 0) (Number -1))) ; Allow 0+ matches
		(Glob "$middle") "no-middle"))

(format #t "type-zero-match ~A\n" type-zero-match)
(test-assert "type-zero-match" (equal? type-zero-match simple-pair))

;-----------------
;; Typed Glob; requires zero or more matches
(format #t "================================== type-middle-match\n")
(define type-middle-match
	(harness
		(TypedVariable (Glob "$middle")
			(Interval (Number 0) (Number -1))) ; Allow 0+ matches
		(Glob "$middle") "have-middle"))

(format #t "type-middle-match ~A\n" type-middle-match)
(test-assert "type-middle-match" (equal? type-middle-match simple-pair))

;-----------------
(test-end tname)

(opencog-test-end)
