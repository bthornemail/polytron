;; ============================================================
;; LEXER.LISP - Meta-Circular Lexer for Constitutional Notation
;; 
;; Lexes raw bytes into (channel . value) tokens
;; Works on same symbol substrate it outputs
;; 
;; Channels: FS=0x1C, GS=0x1D, RS=0x1E, US=0x1F
;; COBS: 0x00 advances channel, 0x01 escapes
;; Braille: 0x2801-0x2808 switch channels
;; ============================================================

;; ============================================================
;;; Channel Constants
;;; ============================================================

(define CHANNEL-FS #x1C)  ;; File Separator
(define CHANNEL-GS #x1D)  ;; Group Separator  
(define CHANNEL-RS #x1E)  ;; Record Separator
(define CHANNEL-US #x1F)  ;; Unit Separator

(define all-channels (list CHANNEL-FS CHANNEL-GS CHANNEL-RS CHANNEL-US))

;; Channel name lookup
(define (channel-name ch)
  (cond 
    ((= ch CHANNEL-FS) "FS")
    ((= ch CHANNEL-GS) "GS")
    ((= ch CHANNEL-RS) "RS")
    ((= ch CHANNEL-US) "US")
    (else "UNKNOWN")))

;;; ============================================================
;;; COBS Constants
;;; ============================================================

(define COBS-DELIM #x00)   ;; Delimiter - advances channel
(define COBS-ESCAPE #x01)   ;; Escape - preserves channel

;;; ============================================================
;;; Braille Channel Markers
;;; ============================================================

(define bar-FS #x2801)  ;; ⠁ - dot 1 = FS
(define bar-GS #x2802)  ;; ⠂ - dot 2 = GS
(define bar-RS #x2804)  ;; ⠄ - dot 3 = RS
(define bar-US #x2808)  ;; ⠈ - dot 4 = US

;; Braille to channel mapping
(define (bar-for-channel bar)
  (cond 
    ((= bar bar-FS) CHANNEL-FS)
    ((= bar bar-GS) CHANNEL-GS)
    ((= bar bar-RS) CHANNEL-RS)
    ((= bar bar-US) CHANNEL-US)
    (else #f)))

;; Channel to braille mapping
(define (channel-for-bar ch)
  (cond
    ((= ch CHANNEL-FS) bar-FS)
    ((= ch CHANNEL-GS) bar-GS)
    ((= ch CHANNEL-RS) bar-RS)
    ((= ch CHANNEL-US) bar-US)
    (else #f)))

;;; ============================================================
;;; Token Constructors
;;; ============================================================

(define (make-token channel value)
  (cons channel value))

(define (token-channel token)
  (car token))

(define (token-value token)
  (cdr token))

;;; ============================================================
;;; Channel Advancement
;;; ============================================================

;; FS -> GS -> RS -> US -> GS (loops back, not to FS)
(define (next-channel ch)
  (cond
    ((= ch CHANNEL-FS) CHANNEL-GS)
    ((= ch CHANNEL-GS) CHANNEL-RS)
    ((= ch CHANNEL-RS) CHANNEL-US)
    ((= ch CHANNEL-US) CHANNEL-GS)
    (else CHANNEL-GS)))

;;; ============================================================
;;; Separator Detection
;;; ============================================================

(define (is-separator b)
  (or (= b #x1C) (= b #x1D) (= b #x1E) (= b #x1F)))

(define (decode-separator b)
  (cond
    ((= b #x1C) CHANNEL-FS)
    ((= b #x1D) CHANNEL-GS)
    ((= b #x1E) CHANNEL-RS)
    ((= b #x1F) CHANNEL-US)
    (else #f)))

;;; ============================================================
;;; Main Lexer
;;; ============================================================

;; lex :: bytes -> tokens
;; Takes a list of bytes, produces a list of (channel . value) tokens
(define (lex bytes)
  (lex-loop bytes '() CHANNEL-GS))

;; lex-loop :: bytes tokens channel -> tokens
;; Internal lexer loop with state
(define (lex-loop bytes tokens current-channel)
  (cond
    ;; Base case: no more bytes
    ((null? bytes) (reverse tokens))
    
    ;; COBS delimiter: 0x00 advances to next channel
    ((= (car bytes) COBS-DELIM)
     (lex-loop (cdr bytes) tokens (next-channel current-channel)))
    
    ;; COBS escape: 0x01 keeps same channel for next byte
    ((= (car bytes) COBS-ESCAPE)
     (if (null? (cdr bytes))
         (lex-loop '() tokens current-channel)
         (lex-loop (cddr bytes) 
                   (cons (make-token current-channel (car (cdr bytes))) 
                         tokens)
                   current-channel)))
    
    ;; Braille channel markers
    ((= (car bytes) bar-FS)
     (lex-loop (cdr bytes) tokens CHANNEL-FS))
    ((= (car bytes) bar-GS)
     (lex-loop (cdr bytes) tokens CHANNEL-GS))
    ((= (car bytes) bar-RS)
     (lex-loop (cdr bytes) tokens CHANNEL-RS))
    ((= (car bytes) bar-US)
     (lex-loop (cdr bytes) tokens CHANNEL-US))
    
    ;; ASCII separator: explicit channel switch
    ((is-separator (car bytes))
     (let ((ch (decode-separator (car bytes))))
       (if ch
           (lex-loop (cdr bytes) tokens ch)
           (lex-loop (cdr bytes) tokens current-channel))))
    
    ;; Default: data token with current channel
    (else
     (lex-loop (cdr bytes)
               (cons (make-token current-channel (car bytes)) tokens)
               current-channel))))

;;; ============================================================
;;; Token Stream Operations
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
    (cons 'FS (length (filter-channel tokens CHANNEL-FS)))
    (cons 'GS (length (filter-channel tokens CHANNEL-GS)))
    (cons 'RS (length (filter-channel tokens CHANNEL-RS)))
    (cons 'US (length (filter-channel tokens CHANNEL-US)))))

;;; ============================================================
;;; Stream Reconstruction
;;; ============================================================

;; Reconstruct byte stream from tokens
(define (unlex tokens)
  (define (build tks result last-ch)
    (cond
      ((null? tks) (reverse result))
      (else
       (let ((ch (token-channel (car tks)))
            (val (token-value (car tks))))
         (cond
           ;; Channel change needed
           ((not (= ch last-ch))
            (let ((cobs-needed (null? (memq ch (list last-ch (next-channel last-ch))))))
                  (sep-needed (is-separator ch)))
              (cond
                (cobs-needed
                 (build (cdr tks) 
                        (cons val (cons COBS-DELIM result))
                        ch))
                (sep-needed
                 (build (cdr tks)
                        (cons val (cons ch result))
                        ch))
                (else
                 (build (cdr tks)
                        (cons val result)
                        ch))))
           ;; Same channel
           (else
            (build (cdr tks)
                   (cons val result)
                   ch)))))))
  (build (reverse tokens) '() #f))

;;; ============================================================
;;; Example Lexing
;;; ============================================================

;; Example: lex a simple packet
;; Input: [GS, A, B, COBS, RS, C, COBS, US, D]
;; Output: tokens with proper channel assignments

(define example-input 
  (list #x1D #x41 #x42 #x00 #x1E #x43 #x00 #x1F #x44))

(define example-tokens (lex example-input))

;; Debug: print token summary
(define (print-token-summary tokens)
  (display "=== TOKEN SUMMARY ===") (newline)
  (display "Total tokens: ") (display (length tokens)) (newline)
  (for-each (lambda (p)
              (display (car p)) (display ": ") 
              (display (cdr p)) (display " tokens") (newline))
            (count-by-channel tokens))
  (newline))

;;; ============================================================
;;; Exports
;;; ============================================================

(export lex lex-loop)
(export next-channel is-separator decode-separator)
(export filter-channel token-values count-by-channel)
(export print-token-summary)
(export make-token token-channel token-value)
(export CHANNEL-FS CHANNEL-GS CHANNEL-RS CHANNEL-US all-channels)
(export channel-name)
(export COBS-DELIM COBS-ESCAPE)
(export bar-FS bar-GS bar-RS bar-US bar-for-channel channel-for-bar)
(export unlex)