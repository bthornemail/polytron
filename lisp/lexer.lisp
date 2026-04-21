;; ============================================================
;; LEXER.LISP - Meta-Circular Lexer for Constitutional Notation
;; 
;; Lexes raw bytes into (channel . value) tokens
;; Works on same symbol substrate it outputs
;; ============================================================

(load "token.h")

;;; ============================================================
;;; LEXER CORE
;;; ============================================================

;; Main lexer: takes list of bytes, produces list of tokens
(define (lex bytes)
  (lex-loop bytes '() CHANNEL-GS))

;; Internal lexer loop with state
(define (lex-loop bytes tokens current-channel)
  (cond
    ((null? bytes) (reverse tokens))
    
    ;; COBS delimiter: 0x00 switches to next channel
    ((= (car bytes) COBS-DELIM)
     (let ((next-ch (next-channel current-channel)))
       (lex-loop (cdr bytes) tokens next-ch)))
    
    ;; COBS escape: 0x01 toggles control mode
    ((= (car bytes) COBS-ESCAPE)
     (lex-loop (cdr bytes) tokens current-channel))
    
    ;; ASCII separator: explicit channel switch
    ((is-separator (car bytes))
     (let ((ch (decode-separator (car bytes))))
       (lex-loop (cdr bytes) tokens ch)))
    
    ;; Check for Braille pattern (0x2800-0x28FF range)
    ((and (>= (car bytes) #x2800) (<= (car bytes) #x28FF))
     (let ((bar (car bytes))
           (ch (channel-for-bar bar)))
       (if ch
           (lex-loop (cdr bytes) tokens ch)
           ;; Not a bar, treat as data
           (let ((token (make-token current-channel (car bytes))))
             (lex-loop (cdr bytes) (cons token tokens) current-channel)))))
    
    ;; Default: treat as Aegean value (star)
    (else
     (let ((token (make-token current-channel (car bytes))))
       (lex-loop (cdr bytes) (cons token tokens) current-channel))))))

;; Helper: advance to next channel (FS -> GS -> RS -> US -> FS)
(define (next-channel ch)
  (cond
    ((= ch CHANNEL-FS) CHANNEL-GS)
    ((= ch CHANNEL-GS) CHANNEL-RS)
    ((= ch CHANNEL-RS) CHANNEL-US)
    ((= ch CHANNEL-US) CHANNEL-FS)
    (else CHANNEL-GS)))

;; Helper: check if byte is a separator
(define (is-separator b)
  (or (= b #x1C) (= b #x1D) (= b #x1E) (= b #x1F)))

;; Helper: decode ASCII separator to channel
(define (decode-separator b)
  (cond
    ((= b #x1C) CHANNEL-FS)
    ((= b #x1D) CHANNEL-GS)
    ((= b #x1E) CHANNEL-RS)
    ((= b #x1F) CHANNEL-US)
    (else #f)))

;;; ============================================================
;;; TOKEN STREAM OPERATIONS
;;; ============================================================

;; Filter tokens by channel
(define (filter-channel tokens channel)
  (filter (lambda (tok) (= (token-channel tok) channel)) tokens))

;; Extract values from tokens
(define (token-values tokens)
  (map token-value tokens))

;; Count tokens per channel
(define (count-by-channel tokens)
  (list
    (cons CHANNEL-FS (length (filter-channel tokens CHANNEL-FS)))
    (cons CHANNEL-GS (length (filter-channel tokens CHANNEL-GS)))
    (cons CHANNEL-RS (length (filter-channel tokens CHANNEL-RS)))
    (cons CHANNEL-US (length (filter-channel tokens CHANNEL-US)))))

;;; ============================================================
;;; EXAMPLE LEXING
;;; ============================================================

;; Example: lex a simple packet
;; Input: [0x00, FS, Aegean0, Aegean1, GS, Aegean2, RS, Aegean3, US, Aegean4, 0x00]
;; Output: tokens with proper channel assignments

(define example-packet
  (list #x00 
        #x1C (aegean-encode 0) (aegean-encode 1)
        #x1D (aegean-encode 2)
        #x1E (aegean-encode 3) (aegean-encode 4)
        #x1F (aegean-encode 5)
        #x00))

(define example-tokens (lex example-packet))

;; Debug: print token summary
(define (print-token-summary tokens)
  (display "=== TOKEN SUMMARY ===") (newline)
  (for-each (lambda (ch)
              (display (channel-name ch))
              (display ": ")
              (display (length (filter-channel tokens ch)))
              (display " tokens")
              (newline))
            all-channels))

;; Run example
;; (print-token-summary example-tokens)

;;; ============================================================
;;; EXPORTS
;;; ============================================================

(export lex lex-loop)
(export next-channel is-separator decode-separator)
(export filter-channel token-values count-by-channel)
(export print-token-summary)