;
; math-library.scm -- Test assorted elementary arithmetic functions
;
; To run by hand, just say `guile -s math-library.scm`.
;

(use-modules (srfi srfi-1))
(use-modules (opencog) (opencog exec))
(use-modules (opencog test-runner))

(opencog-test-runner)
(define tname "math-library-test")
(test-begin tname)

; -----------------------------------------------
; Test Log2Link

(test-assert "log2"
	(equal? (Number 0 1 2 3 4 5)
		(cog-execute! (Log2 (Number 1 2 4 8 16 32)))))

; -----------------------------------------------
; Test PowLink
(test-assert "eight"
	(equal? (Number 8) (cog-execute! (Pow (Number 2) (Number 3)))))

(test-assert "pow three"
	(equal? (Number 8 27 64 125)
		(cog-execute! (Pow (Number 2 3 4 5) (Number 3)))))

(test-assert "two pow"
	(equal? (NumberNode 2 4 8 16)
		(cog-execute! (Pow (Number 2) (Number 1 2 3 4)))))

(test-assert "x**x"
	(equal? (NumberNode 1 4 27 256)
		(cog-execute! (Pow (Number 1 2 3 4) (Number 1 2 3 4)))))

(for-each
	(lambda (n)
		(define rp (cog-execute! (Pow (Number 2)
			(RandomNumber (Number 2) (Number 3)))))

		(define rn (car (cog-value->list rp)))
		(test-assert "2**random"
			(and (<= 4.0 rn) (<= rn 8.0))))
	(iota 30))

; -----------------------------------------------
; Test SineLink

(define pi 3.14159265358979)

(test-assert "sine n pi"
	(equal? (Number 0 0 0 0 0)
		(cog-execute! (Sine (Number 0 pi (* 2 pi) (* 3 pi) (* 4 pi))))))

(define (npih n) (* n 0.5 pi))

(test-assert "sine n pi-half"
	(equal? (Number 1 -1 1 -1 1)
		(cog-execute! (Sine (Number
			(npih 1) (npih 3) (npih 5) (npih 7) (npih 9))))))

; -----------------------------------------------
; Test Heaviside and SineLink

(for-each
	(lambda (n)
		(define sn (cog-execute! (Heaviside (Sine (Number n)))))

		(define wv (modulo (floor (/ n pi)) 2))
		(test-assert "square-wave" (equal? sn wv))
	)
	(iota 70))

(test-end tname)

(opencog-test-end)
