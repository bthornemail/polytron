;; ============================================================
;; TOKEN.H - Token Representation for Constitutional Notation
;; Pairwise Lisp with channel.value pairs
;; ============================================================

;; A token is a pair: (channel . value)
;; channel: 0x1C (FS), 0x1D (GS), 0x1E (RS), 0x1F (US)
;; value:   16-bit Aegean numeral

;;; Token constructors
(define (make-token channel value)
  (cons channel value))

(define (token-channel token)
  (car token))

(define (token-value token)
  (cdr token))

;;; Channel constants (ASCII separators)
(define CHANNEL-FS #x1C)  ;; File Separator
(define CHANNEL-GS #x1D)  ;; Group Separator  
(define CHANNEL-RS #x1E)  ;; Record Separator
(define CHANNEL-US #x1F)  ;; Unit Separator

;;; Channel list for iteration
(define all-channels 
  (list CHANNEL-FS CHANNEL-GS CHANNEL-RS CHANNEL-US))

;;; Channel name lookup
(define (channel-name ch)
  (cond 
    ((= ch CHANNEL-FS) "FS")
    ((= ch CHANNEL-GS) "GS")
    ((= ch CHANNEL-RS) "RS")
    ((= ch CHANNEL-US) "US")
    (else "UNKNOWN")))

;;; Mode: orientation and chirality
;;; Mode is a pair: (orientation . chirality)
;;; orientation: 0 = identity, 1 = reverse
;;; chirality: 0 = right-handed (BOM FEFF), 1 = left-handed (BOM FFFE)

(define mode-XX (cons 0 0))   ;; identity, right-handed
(define mode-Xx (cons 0 1))   ;; identity, left-handed
(define mode-xX (cons 1 0))   ;; reverse, right-handed
(define mode-xx (cons 1 1))   ;; reverse, left-handed

(define current-mode mode-XX)

(define (mode-orientation m) (car m))
(define (mode-chirality m) (cdr m))

;;; BOM constants
(define BOM-FEFF #xFEFF)  ;; Big Endian
(define BOM-FFFE #xFFFE)  ;; Little Endian

(define (bom-for-mode mode)
  (if (= (mode-chirality mode) 0) BOM-FEFF BOM-FFFE))

;;; COBS framing
(define COBS-DELIM #x00)
(define COBS-ESCAPE #x01)

;; Token list structure
(define empty-tokens '())

(define (add-token tokens token)
  (cons token tokens))

(define (reverse-tokens tokens)
  (reverse tokens))

;;; ============================================================
;;; AEGEAN NUMERALS (Values/Stars)
;;; ============================================================

;; Aegean numeral encoding table
;; Unicode U+10100-U+10109 for values 0-9
(define aegean-table
  (list 
    (cons 0 #x10100)   ;; 𐄀
    (cons 1 #x10101)   ;; 𐄇
    (cons 2 #x10102)   ;; 𐄈
    (cons 3 #x10103)   ;; 𐄉
    (cons 4 #x10104)   ;; 𐄊
    (cons 5 #x10105)   ;; 𐄋
    (cons 6 #x10106)   ;; 𐄌
    (cons 7 #x10107)   ;; 𐄍
    (cons 8 #x10108)   ;; 𐄎
    (cons 9 #x10109))) ;; 𐄏

(define (aegean-encode n)
  (if (< n 10)
      (cdr (assoc n aegean-table))
      #f))

(define (aegean-decode code)
  (car (rassoc code aegean-table)))

;;; ============================================================
;;; BRAILLE PATTERNS (Channel Bars)
;;; ============================================================

;; 6-dot Braille encoding (dots 1-6)
;; Each dot position adds 2^(n-1) to base 0x2800
(define (braille-6dot d1 d2 d3 d4 d5 d6)
  (+ #x2800
     (* d1 1)
     (* d2 2)
     (* d3 4)
     (* d4 8)
     (* d5 16)
     (* d6 32)))

;; 8-dot Braille encoding (dots 1-8)
(define (braille-8dot d1 d2 d3 d4 d5 d6 d7 d8)
  (+ #x2800
     (* d1 1)
     (* d2 2)
     (* d3 4)
     (* d4 8)
     (* d5 16)
     (* d6 32)
     (* d7 64)
     (* d8 128)))

;; Channel bars as Braille
;; Dot 1 = FS, Dot 2 = GS, Dot 3 = RS, Dot 4 = US
(define bar-FS (braille-6dot 1 0 0 0 0 0))   ;; ⠁
(define bar-GS (braille-6dot 0 1 0 0 0 0))   ;; ⠂
(define bar-RS (braille-6dot 0 0 1 0 0 0))   ;; ⠄
(define bar-US (braille-6dot 0 0 0 1 0 0))   ;; ⠈

(define (bar-for-channel channel)
  (cond 
    ((= channel CHANNEL-FS) bar-FS)
    ((= channel CHANNEL-GS) bar-GS)
    ((= channel CHANNEL-RS) bar-RS)
    ((= channel CHANNEL-US) bar-US)
    (else #x2800)))

(define (channel-for-bar bar)
  (cond
    ((= bar bar-FS) CHANNEL-FS)
    ((= bar bar-GS) CHANNEL-GS)
    ((= bar bar-RS) CHANNEL-RS)
    ((= bar bar-US) CHANNEL-US)
    (else #f)))

;;; ============================================================
;;; PRECISION FORMATS (Stars with weights)
;;; ============================================================

(define PREC-HALF    '(1 . 16))    ;; 16-bit half precision
(define PREC-SINGLE  '(2 . 32))    ;; 32-bit single precision
(define PREC-DOUBLE '(3 . 64))    ;; 64-bit double precision
(define PREC-QUAD   '(4 . 128))   ;; 128-bit quadruple
(define PREC-OCT    '(5 . 256))   ;; 256-bit octuple

(define (precision-weight p) (car p))
(define (precision-bits p)  (cdr p))

;;; ============================================================
;;; SEXAGESIMAL POSITIONS (from TypesOnTypes.md)
;;; ============================================================

;; Position order: 60^position
(define position-quadprime   -4)   ;; 60^-4
(define position-tripleprime -3)   ;; 60^-3
(define position-doubleprime -2)   ;; 60^-2
(define position-prime       -1)   ;; 60^-1
(define position-degree       0)   ;; 60^0 (root)
(define position-minute       1)   ;; 60^1
(define position-second       2)   ;; 60^2
(define position-third       3)   ;; 60^3
(define position-fourth       4)   ;; 60^4

(define (position-name pos)
  (cond
    ((= pos -4) "quadprime")
    ((= pos -3) "tripleprime")
    ((= pos -2) "doubleprime")
    ((= pos -1) "prime")
    ((= pos 0)  "degree")
    ((= pos 1)  "minute")
    ((= pos 2)  "second")
    ((= pos 3)  "third")
    ((= pos 4)  "fourth")
    (else "unknown")))

;; Slot: position + coefficient
(define (make-slot position coefficient)
  (cons position coefficient))

(define (slot-position s) (car s))
(define (slot-coefficient s) (cdr s))

;; Sexagesimal term: sex60(term, left, unit, right)
;; left:  list of slots (negative positions)
;; right: list of slots (positive positions)
(define (make-sex60 left unit right)
  (list left unit right))

(define (sex60-left s60)  (car s60))
(define (sex60-unit s60)  (cadr s60))
(define (sex60-right s60) (caddr s60))

;; Evaluate sex60 to numeric value
(define (evaluate-sex60 s60)
  (let* ((left  (sex60-left s60))
         (unit  (sex60-unit s60))
         (right (sex60-right s60))
         (left-val (eval-slot-list left))
         (right-val (eval-slot-list right)))
    (+ left-val unit right-val)))

(define (eval-slot-list slots)
  (if (null? slots)
      0
      (+ (* (slot-coefficient (car slots))
            (expt 60 (slot-position (car slots))))
         (eval-slot-list (cdr slots))))))

;;; ============================================================
;;; EXPORTS
;;; ============================================================

(export make-token token-channel token-value)
(export CHANNEL-FS CHANNEL-GS CHANNEL-RS CHANNEL-US all-channels)
(export channel-name)
(export mode-XX mode-Xx mode-xX mode-xx current-mode)
(export mode-orientation mode-chirality)
(export BOM-FEFF BOM-FFFE bom-for-mode)
(export COBS-DELIM COBS-ESCAPE)
(export aegean-encode aegean-decode)
(export braille-6dot braille-8dot)
(export bar-FS bar-GS bar-RS bar-US)
(export bar-for-channel channel-for-bar)
(export PREC-HALF PREC-SINGLE PREC-DOUBLE PREC-QUAD PREC-OCT)
(export precision-weight precision-bits)
(export position-quadprime position-tripleprime position-doubleprime)
(export position-prime position-degree position-minute position-second)
(export position-name)
(export make-slot slot-position slot-coefficient)
(export make-sex60 sex60-left sex60-unit sex60-right evaluate-sex60)