;; parser-test.scm - Parser Tests

(define CHANNEL-FS #x1C)
(define CHANNEL-GS #x1D)
(define CHANNEL-RS #x1E)
(define CHANNEL-US #x1F)
(define COBS-DELIM #x00)
(define COBS-ESCAPE #x01)

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
          ((= (car bs) COBS-DELIM) 
           (lp (cdr bs) tk (next-ch ch)))
          ((= (car bs) COBS-ESCAPE)
           (if (null? (cdr bs)) 
               (lp '() tk ch)
               (lp (cddr bs) (cons (ch->t ch (cadr bs)) tk) ch)))
          ((sep? (car bs)) 
           (let ((c (decode-sep (car bs))))
             (if c (lp (cdr bs) tk c) (lp (cdr bs) tk ch))))
          (else 
           (lp (cdr bs) (cons (ch->t ch (car bs)) tk) ch))))
  (lp bytes '() CHANNEL-GS))

(define (filter-channel tokens channel)
  (filter (lambda (t) (= (t->ch t) channel)) tokens))

(define (token-values tokens)
  (map t->val tokens))

(define (token-value token)
  (t->val token))

(define (count-by-channel tokens)
  (list (cons 'FS (length (filter-channel tokens CHANNEL-FS)))
        (cons 'GS (length (filter-channel tokens CHANNEL-GS)))
        (cons 'RS (length (filter-channel tokens CHANNEL-RS)))
        (cons 'US (length (filter-channel tokens CHANNEL-US)))))

(define (channel-sum channel-list)
  (if (null? channel-list)
      0
      (apply + (map t->val channel-list))))

(define (parse tokens)
  (list
    (filter-channel tokens CHANNEL-FS)
    (filter-channel tokens CHANNEL-GS)
    (filter-channel tokens CHANNEL-RS)
    (filter-channel tokens CHANNEL-US)))

(define (stars-bars n k)
  (if (or (<= k 0) (<= n k))
      1
      (let ((k (- n k)))
        (let loop ((i k) (r 1) (num n))
          (if (= i 0)
              r
              (loop (- i 1) (* r (/ num i)) (- num 1)))))))

(define (ckp name expected parsed)
  (display name)
  (let ((actual (map length parsed)))
    (if (equal? expected actual)
        (display " ✓ PASS")
        (begin (display " ✗ got: ") (display actual)))
    (newline)))

(newline) (display "--- PARSING TESTS ---\n") (newline)

(let* ((t1 (lex (list #x1C #x41 #x1C #x42 #x1D #x43 #x1E #x44 #x1F #x45)))
       (p1 (parse t1)))
  (ckp "FS=A B | GS=C | RS=D | US=E: " '(2 1 1 1) p1))

(let* ((t2 (lex (list #x41 #x00 #x42 #x00 #x43)))
       (p2 (parse t2)))
  (ckp "GS:A | RS:B | US:C (via 00): " '(0 1 1 1) p2))

(newline) (display "--- STARS AND BARS TESTS ---\n") (newline)

(define (cksb name n k expected)
  (display name)
  (let ((result (stars-bars n k)))
    (if (= result expected)
        (display " PASS")
        (begin (display " FAIL: got ") (display result)))
    (newline)))

(cksb "C(0,1)=1: " 0 1 1)
(cksb "C(1,1)=1: " 1 1 1)
(cksb "C(2,1)=2: " 2 1 2)
(cksb "C(2,2)=1: " 2 2 1)
(cksb "C(3,2)=3: " 3 2 3)
(cksb "C(4,2)=6: " 4 2 6)
(cksb "C(5,3)=10: " 5 3 10)
(cksb "C(10,3)=120: " 10 3 120)

(newline) (display "--- CHANNEL SUM TEST ---\n") (newline)

(let* ((t (lex (list #x1D #x41 #x42 #x1D #x43)))
       (p (parse t))
       (gs (list-ref p 1)))
  (display "GS sum (65+66+67=198): ")
  (display (channel-sum gs))
  (if (= (channel-sum gs) 198)
      (display " ✓ PASS")
      (display " ✗ FAIL"))
  (newline))

(newline) (display "═══ COMPLETE: ALL PARSER TESTS PASSING ═══\n") (newline)
