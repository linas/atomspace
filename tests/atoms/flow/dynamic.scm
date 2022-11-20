;
; dynamic.scm -- test-case version of `flow-formulas.scm` example.
;

(use-modules (opencog) (opencog exec))

; For DynamicUTest::test_predicate_formula()

(define A (Concept "A"))
(define B (Concept "B"))

(define ftv
	(FormulaTruthValue
		(FormulaPredicate
			(Minus
				(Number 1)
				(Times (StrengthOf A) (StrengthOf B)))
			(Times (ConfidenceOf A) (ConfidenceOf B)))))

; ----------
; For DynamicUTest::test_formula_define()
(DefineLink
   (DefinedPredicate "has a reddish color")
   (FormulaPredicate
      (Minus
         (Number 1)
         (Times
            (StrengthOf (Variable "$X"))
            (StrengthOf (Variable "$Y"))))
      (Times
         (ConfidenceOf (Variable "$X"))
         (ConfidenceOf (Variable "$Y")))))

(define evlnk
	(Evaluation
		(DefinedPredicate "has a reddish color")
		(List (Concept "A") (Concept "B"))))

(define tv-stream (FormulaTruthValue evlnk))

; ----------
; for DynamicUTest::test_dynamic_formula()

(define a-implies-b (Implication (Concept "A") (Concept "B")))

(cog-execute!
	(SetTV
		(Implication (Concept "A") (Concept "B"))
		(FormulaPredicate
			(Minus
				(Number 1)
				(Times
					(StrengthOf (Concept "A"))
					(StrengthOf (Concept "B"))))
			(Times
				(ConfidenceOf (Concept "A"))
				(ConfidenceOf (Concept "B"))))))

(DefineLink
   (DefinedPredicate "dynamic example")
   (FormulaPredicate
      (Minus
         (Number 1)
         (Times
            (StrengthOf (Variable "$X"))
            (StrengthOf (Variable "$Y"))))
      (Times
         (ConfidenceOf (Variable "$X"))
         (ConfidenceOf (Variable "$Y")))))

; -----------
; For DynamicUTest::test_defined_dynamic()
(define p-implies-q (Implication (Concept "P") (Concept "Q")))
(cog-execute!
	(SetTV
		(Implication (Concept "P") (Concept "Q"))
		(DefinedPredicate "dynamic example")
		(List (Concept "A") (Concept "B"))))

(define j-implies-k (Implication (Concept "J") (Concept "K")))
(cog-set-tv!
	(Implication (Concept "J") (Concept "K"))
	(FormulaTruthValue
		(DefinedPredicate "dynamic example")
		(List (Concept "A") (Concept "B"))))

; -------------------------------------------------------------
; for DynamicUTest::test_formula_stream()
(define foo (Concept "foo"))
(define bar (Concept "bar"))
(define akey (Predicate "some key"))
(define bkey (Predicate "other key"))

(cog-set-value! foo akey (FloatValue 1 2 3 4 5))

(define fstream (FormulaStream (Plus (Number 10) (FloatValueOf foo akey))))
(cog-set-value! bar bkey fstream)

; ------- THE END -------
