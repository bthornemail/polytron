;; ============================================================
;; PARSER.LISP - Stars and Bars Channel Grouper
;;
;; Groups tokens by channel into 4 channel lists
;; Implements stars and bars combinatorics
;; ============================================================

(load "token.h")
(load "lexer.lisp")

;;; ============================================================
;;; PARSER: CHANNEL GROUPING
;;; ============================================================

;; Parse: takes lexed tokens, returns 4 channel lists
;; Result: (fs-list gs-list rs-list us-list)
(define (parse tokens)
  (list
    (filter-channel tokens CHANNEL-FS)
    (filter-channel tokens CHANNEL-GS)
    (filter-channel tokens CHANNEL-RS)
    (filter-channel tokens CHANNEL-US)))

;; Parse and extract values only
(define (parse-values tokens)
  (map token-values (parse tokens)))

;; Parse and get counts
(define (parse-counts tokens)
  (count-by-channel tokens))

;;; ============================================================
;;; STARS AND BARS CALCULATIONS
;;; ============================================================

;; Stars and Bars theorem:
;; C(n+k-1, k-1) = combinations of n indistinguishable items into k bins

;; Calculate number of ways to distribute n stars into k bars
(define (stars-bars-combinations n k)
  (if (<= k 1)
      1
      (factorial (+ n k -1) (factorial k-1 (factorial n))))

;; Factorial helper
(define (factorial n)
  (if (<= n 1)
      1
      (* n (factorial (- n 1)))))

;; Factorial with cancellation for combinations
(define (factorial-div n d)
  (if (or (= d 0) (= n 0))
      1
      (* (/ n d) (factorial-div (- n 1) (- d 1)))))

;; Compute stars-bars more efficiently
(define (combinations n k)
  (if (or (= k 0) (= k n))
      1
      (if (> k (- n k))
          (combinations (- n k) k)
          (let ((result 1))
            (for i 1 k
              (set! result (* result (/ (+ n -1 i) i)))
            result))))

;;; ============================================================
;;; CHANNEL PROPERTIES
;;; ============================================================

;; Compute total stars (sum of all values)
(define (total-stars tokens)
  (apply + (token-values tokens)))

;; Compute channel weight (importance based on token count)
(define (channel-weight channel-list)
  (length channel-list))

;; Compute channel sum (sum of values in channel)
(define (channel-sum channel-list)
  (if (null? channel-list)
      0
      (apply + (map token-value channel-list))))

;; Get channel statistics
(define (channel-stats tokens)
  (let* ((parsed (parse tokens))
         (fs (list-ref parsed 0))
         (gs (list-ref parsed 1))
         (rs (list-ref parsed 2))
         (us (list-ref parsed 3)))
    (list
      (list 'FS (length fs) (channel-sum fs))
      (list 'GS (length gs) (channel-sum gs))
      (list 'RS (length rs) (channel-sum rs))
      (list 'US (length us) (channel-sum us)))))

;;; ============================================================
;;; EXAMPLE PARSING
;;; ============================================================

;; Parse example tokens
(define parsed-example (parse example-tokens))

(define (print-parsed parsed)
  (display "=== PARSED CHANNELS ===") (newline)
  (let ((names '("FS" "GS" "RS" "US")))
    (for-each (lambda (name ch)
                (display name)
                (display ": ")
                (display (length ch))
                (display " tokens, sum=")
                (display (channel-sum ch))
                (newline))
              names parsed)))

;; Run
;; (print-parsed parsed-example)

;;; ============================================================
;;; EXPORTS
;;; ============================================================

(export parse parse-values parse-counts)
(export stars-bars-combinations combinations)
(export total-stars channel-weight channel-sum channel-stats)
(export print-parsed)