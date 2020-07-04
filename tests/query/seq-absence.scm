;;
;; seq-absence.scm
;;
;; Test the AbsentLink inside a SequentialAndLink.
;;
;; Check to see if the atomspace does NOT contain the link
;;    (Evaluation
;;        (Predicate "visible")
;;        (List (Concept "anything")))
;;
; ------------------------------------------------------

(use-modules (opencog) (opencog exec))

; Unit test makes the PresentLink true
(define or-visible-put
	(Satisfaction
		(TypedVariable (Variable "$x") (Type 'Concept))
		;; SequentialOr - verify predicates in sequential order.
		(SequentialOr
			(Present (Evaluation (Predicate "yes-visible")
					(List (Variable "$x"))))
			;; If above fails then set state
			(True (Put
					(State (Anchor "state") (Variable "$yy"))
					(Concept "ohhh noot visible")))
		)))

; Same as above, except unit test makes the PresentLink false
(define or-put
	(Satisfaction
		(TypedVariable (Variable "$x") (Type 'Concept))
		;; SequentialOr - verify predicates in sequential order.
		(SequentialOr
			(Present (Evaluation (Predicate "or-visible")
					(List (Variable "$x"))))
			;; If above fails then set state
			(True (Put
					(State (Anchor "state") (Variable "$yy"))
					(Concept "not-vis")))
		)))


(define trig 0)
(define (incr-trig) (set! trig (+ trig 1)) (stv 1 1))

(define or-presence
	(Satisfaction
		(TypedVariable (Variable "$x") (Type 'Concept))
		;; SequentialOrLink - verify predicates in sequential order.
		(SequentialOr
			(Present (Evaluation (Predicate "visible")
					(List (Variable "$x"))))
			;; If above fails then increment
			(Evaluation (GroundedPredicate "scm: incr-trig") (List))
		)))

(define and-absence
	(Satisfaction
		(TypedVariable (Variable "$x") (Type 'Concept))
		(SequentialAnd
			(Absent (Evaluation (Predicate "visible")
					(List (Variable "$x"))))
			;; If above fails then increment
			(Evaluation (GroundedPredicate "scm: incr-trig") (List))
		)))

;; You might think that this one is similar to the above, but the
;; Pattern matcher treates present and Absent links in a very
;; different way. This one actually does not work correctly,
;; and right now, I'm not gonna fix it... XXX FIXME.
(define and-not-present
	(Satisfaction
		(TypedVariable (Variable "$x") (Type 'Concept))
		(SequentialAnd
			(Not (Present (Evaluation (Predicate "visible")
					(List (Variable "$x")))))
			;; If above fails then increment
			(Evaluation
				(GroundedPredicate "scm: incr-trig") (List))
		)))

;; This one is a bit perverted, and similar expressions are guaranteed
;; to fail in the general case, cause the pattern matcher treats
;; AbsentLinks in a fundamentally different way. But we test this
;; anyway, for now.  Like the above, its currently broken. Its too
;; weird right now for me to want to fix it, so I am punting on this.
;; XXX FIXME ... this and the above need to get done right.

(define or-not-absent
	(Satisfaction
		(TypedVariable (Variable "$x") (Type 'Concept))
		(SequentialOr
			(Not (Absent (Evaluation (Predicate "visible")
					(List (Variable "$x")))))
			;; If above fails then increment
			(Evaluation (GroundedPredicate "scm: incr-trig") (List))
		)))

; ------------------------------------------------------
