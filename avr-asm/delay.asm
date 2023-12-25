; ------------------------------------------------------------------------------
; -----   DELAY_MS & DELAY_US                                              -----
; ------------------------------------------------------------------------------
; Assumes that following registers have been set:
;	Fparam0		Compare value low byte 	- for OCR1AL
;	Fparam1		Compare value high byte - for OCR1AH
; And that the following registers have been defined:
;	TMP
; And that following 1-byte variable is in SRAM:
;	statereg
delay_ms:
	push 	TMP
	ldi		TMP, (1 << WGM12) | (1 << CS12) | (1 << CS10)	; CTC, Clk Div 1024
 	rjmp	_delay
 delay_us:
 	push 	TMP
 	ldi		TMP, (1 << WGM12) | (1 << CS10)				; CTC mode. No Clk Div

 _delay:
	push 	Fparam0
	push	Fparam1
.IFDEF CLK_MHZ
	; With a 16MHz clock, need to multiply setting by 16
	push	TMP
	ldi		TMP, CLK_MHZ
	cpi		TMP, 16
	brne	_delay_no_div
	mul		Fparam1, TMP	; Multiply upper byte - puts result in R1:R0
	mov		Fparam1, R0		; Store lower result byte, ignore upper result byte
	mul		Fparam0, TMP	; Multiply lower byte - puts result in R1:R0
	add		Fparam1, R1
	mov 	Fparam0, R0
_delay_no_div:
	pop		TMP
.ENDIF

	; NB: Following two lines must be high-byte first
	sts		OCR1AH, Fparam1	; Load parameters into output control regs, to
	sts		OCR1AL, Fparam0	; set the number at which timer fires interrupt

	sts		TCCR1B, TMP		; Store settings created above into control reg B
	clr		TMP				; Control reg A is going to be all 0s
	sts		TCCR1A, TMP

	ldi		TMP, (1 << OCIE1A)	; Set interrupt enable bit
	sts		TIMSK1, TMP

	; We've set & started the Timer. Now need to wait for it to complete.
_delay_loop:
	lds		TMP, statereg				; Load the current state register
	andi 	TMP, delaydone				; Check the delaydone bit
	breq	_delay_loop					; If zero, not set yet

	lds		TMP, statereg				; Otherwise re-load statereg
	cbr		TMP, delaydone			; Unset the delaydone bit
	sts		statereg, TMP				; And re-store the register

	pop		Fparam1
	pop		Fparam0
	pop		TMP
	ret
