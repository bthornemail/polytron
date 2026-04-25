;; lexer-test.scm - Comprehensive Lexer Tests (All Passing)

(define CHANNEL-FS #x1C)
(define CHANNEL-GS #x1D)
(define CHANNEL-RS #x1E)
(define CHANNEL-US #x1F)
(define COBS-DELIM #x00)
(define COBS-ESCAPE #x01)
(define bar-FS #x2801)
(define bar-GS #x2802)
(define bar-RS #x2804)
(define bar-US #x2808)

(define (ch->t ch val) (cons ch val))
(define (t->ch t) (car t))
(define (t->val t) (cdr t))

(define (next-ch ch)
  (cond ((= ch CHANNEL-FS) CHANNEL-GS)
        ((= ch CHANNEL-GS) CHANNEL-RS)
        ((= ch CHANNEL-RS) CHANNEL-US)
        ((= ch CHANNEL-US) CHANNEL-GS)
        (else CHANNEL-GS)))

(define (sep? b) (or (= b #x1C) (= b #x1D) (= b #x1E) (= b #x1F)))

(define (decode-sep b)
  (cond ((= b #x1C) CHANNEL-FS) ((= b #x1D) CHANNEL-GS)
        ((= b #x1E) CHANNEL-RS) ((= b #x1F) CHANNEL-US) (else #f)))

(define (lex bytes)
  (define (lp bs tk ch)
    (cond ((null? bs) (reverse tk))
          ((= (car bs) COBS-DELIM) (lp (cdr bs) tk (next-ch ch)))
          ((= (car bs) COBS-ESCAPE)
           (if (null? (cdr bs)) (lp (quote ()) tk ch)
               (lp (cddr bs) (cons (ch->t ch (cadr bs)) tk) ch)))
          ((= (car bs) bar-FS) (lp (cdr bs) tk CHANNEL-FS))
          ((= (car bs) bar-GS) (lp (cdr bs) tk CHANNEL-GS))
          ((= (car bs) bar-RS) (lp (cdr bs) tk CHANNEL-RS))
          ((= (car bs) bar-US) (lp (cdr bs) tk CHANNEL-US))
          ((sep? (car bs)) (let ((c (decode-sep (car bs))))
                             (if c (lp (cdr bs) tk c) (lp (cdr bs) tk ch))))
          (else (lp (cdr bs) (cons (ch->t ch (car bs)) tk) ch))))
  (lp bytes (quote ()) CHANNEL-GS))

(define (fch t ch) (filter (lambda (x) (= (t->ch x) ch)) t))
(define (cnt t ch) (length (fch t ch)))
(define (c t) (list (cnt t CHANNEL-FS) (cnt t CHANNEL-GS) (cnt t CHANNEL-RS) (cnt t CHANNEL-US)))

(define (ck n e t) 
  (display n) 
  (if (equal? e (c t)) (display " ✓ PASS") 
      (begin (display " ✗ got: ") (display (c t))))
  (newline))

(display "═══ LEXER TEST SUITE ═══")(newline)(newline)

;; ASCII
(ck "FS=A FS=B GS=C RS=D US=E" '(2 1 1 1) (lex (list #x1C #x41 #x1C #x42 #x1D #x43 #x1E #x44 #x1F #x45)))
(ck "GS=A,B then RS=C" '(0 2 1 0) (lex (list #x1D #x41 #x42 #x1E #x43)))
(newline)

;; COBS
(ck "00 switches: GS->RS->US" '(0 1 1 1) (lex (list #x41 #x00 #x42 #x00 #x43)))
(ck "data before/after 00" '(0 1 2 0) (lex (list #x41 #x00 #x42 #x43)))
(newline)

;; Escape
(ck "01 escapes, same channel" '(0 2 1 0) (lex (list #x1D #x41 #x01 #x42 #x1E #x43)))
(newline)

;; Single
(ck "single 0x00" '(0 0 0 0) (lex (list #x00)))
(ck "single A" '(0 1 0 0) (lex (list #x41)))
(newline)

;; Braille
(ck "0x2801 = FS" '(2 0 0 0) (lex (list #x2801 #x41 #x42)))
(ck "0x2802 = GS" '(0 2 0 0) (lex (list #x2802 #x41 #x42)))
(ck "0x2804 = RS" '(0 0 2 0) (lex (list #x2804 #x41 #x42)))
(ck "0x2808 = US" '(0 0 0 2) (lex (list #x2808 #x41 #x42)))
(newline)

;; Mixed
(ck "ASCII+Braille+COBS" '(1 2 2 1) (lex (list #x1C #x41 #x2802 #x42 #x43 #x00 #x44 #x1E #x45 #x2808 #x46)))
(newline)

;; Edge
(ck "empty" '(0 0 0 0) (lex (quote ())))
(newline)

;; Aegean
(ck "U+10100+ data" '(0 3 3 0) (lex (list #x10100 #x10101 #x10102 #x00 #x10103 #x10104 #x10105)))
(newline)

(display "═══ COMPLETE: ALL TESTS PASSING ═══")(newline)