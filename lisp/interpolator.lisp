;; ============================================================
;; INTERPOLATOR.LISP - K-Fold Semantic Operation
;;
;; Folds constitutional kernel K over all channel values
;; Computes the final SID (Semantic ID)
;; ============================================================

(load "token.h")
(load "lexer.lisp")
(load "parser.lisp")

;;; ============================================================
;;; CONSTITUTIONAL KERNEL
;;; ============================================================

;; The constitutional kernel K(p, C)
;; K(p,C) = rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C

(define CONSTITUTIONAL-C #x1D)  ;; GS - Group Separator

;; Rotate left
(define (rotl x n)
  (let ((n (modulo n 16)))
    (bitwise-ior (arithmetic-shift x n)
                 (arithmetic-shift x (- n 16)))))

;; Rotate right
(define (rotr x n)
  (let ((n (modulo n 16)))
    (bitwise-ior (arithmetic-shift x (- n))
                 (arithmetic-shift x (- 16 n)))))

;; The kernel
(define (K p C)
  (bitwise-xor (rotl p 1)
               (bitwise-xor (rotl p 3)
                            (bitwise-xor (rotr p 2)
                                         C))))

;; Compute SID from a value
(define (compute-sid value)
  (K value CONSTITUTIONAL-C))

;;; ============================================================
;;; INTERPOLATION: FOUR MODES
;;; ============================================================

;; Interpolate: combine all channels into single SID
;; Based on mode, applies different interpretation

(define (interpolate tokens mode)
  (let ((parsed (parse tokens)))
    (cond
      ((equal? mode mode-XX) (interpret-XX parsed))
      ((equal? mode mode-Xx) (interpret-Xx parsed))
      ((equal? mode mode-xX) (interpret-xX parsed))
      ((equal? mode mode-xx) (interpret-xx parsed))
      (else (interpret-XX parsed)))))

;; Mode XX: left-to-right, big-endian (identity, right-handed)
;; Apply K fold directly
(define (interpret-XX parsed)
  (let* ((fs (list-ref parsed 0))
         (gs (list-ref parsed 1))
         (rs (list-ref parsed 2))
         (us (list-ref parsed 3))
         (seed (channel-sum fs)))
    (let ((after-gs (fold K seed gs)))
      (let ((after-rs (fold K after-gs rs)))
        (fold K after-rs us)))))

;; Mode Xx: left-to-right, little-endian
;; Swap bytes then apply K
(define (interpret-Xx parsed)
  (let* ((fs (list-ref parsed 0))
         (gs (list-ref parsed 1))
         (rs (list-ref parsed 2))
         (us (list-ref parsed 3))
         (seed (swap-bytes (channel-sum fs))))
    (let ((after-gs (fold K seed gs)))
      (let ((after-rs (fold K after-gs rs)))
        (fold K after-rs us)))))

;; Mode xX: right-to-left, big-endian
;; Reverse channel order, apply K
(define (interpret-xX parsed)
  (let* ((rev-parsed (reverse parsed))
         (fs (list-ref rev-parsed 0))
         (gs (list-ref rev-parsed 1))
         (rs (list-ref rev-parsed 2))
         (us (list-ref rev-parsed 3))
         (seed (channel-sum us)))
    (let ((after-rs (fold K seed rs)))
      (let ((after-gs (fold K after-rs gs)))
        (fold K after-gs fs)))))

;; Mode xx: right-to-left, little-endian
;; Reverse and swap bytes
(define (interpret-xx parsed)
  (let* ((rev-parsed (reverse parsed))
         (fs (list-ref rev-parsed 0))
         (gs (list-ref rev-parsed 1))
         (rs (list-ref rev-parsed 2))
         (us (list-ref rev-parsed 3))
         (seed (swap-bytes (channel-sum us))))
    (let ((after-rs (fold K seed rs)))
      (let ((after-gs (fold K after-rs gs)))
        (fold K after-gs fs)))))

;; Helper: swap bytes (16-bit)
(define (swap-bytes word)
  (let ((low (bitwise-and #xFF word))
        (high (bitwise-and #xFF00 word)))
    (bitwise-ior (arithmetic-shift low 8)
                 (arithmetic-shift high -8))))

;; Fold helper
(define (fold func init list)
  (if (null? list)
      init
      (fold func (func init (car list)) (cdr list))))

;;; ============================================================
;;; EXAMPLE INTERPOLATION
;;; ============================================================

;; Interpolate example tokens
(define example-sid (interpolate example-tokens mode-XX))

(define (print-sid sid mode)
  (display "=== INTERPOLATION RESULT ===") (newline)
  (display "Mode: ") (display mode) (newline)
  (display "SID: 0x") (display (number->string sid 16)) (newline)
  (newline))

;; Run examples
;; (print-sid example-sid mode-XX)

;;; ============================================================
;;; COMPLETE WORKFLOW
;;; ============================================================

;; Full pipeline: bytes -> tokens -> parse -> interpolate -> SID
(define (process-packet bytes mode)
  (let* ((tokens (lex bytes))
         (sid (interpolate tokens mode)))
    sid))

;; Process example
;; (process-packet example-packet mode-XX)

;;; ============================================================
;;; EXPORTS
;;; ============================================================

(export K compute-sid)
(export interpolate)
(export interpret-XX interpret-Xx interpret-xX interpret-xx)
(export swap-bytes)
(export process-packet)