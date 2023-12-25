; ******************************************************************************
; *****  DEBUGGING FUNCTIONS                                               *****
; ******************************************************************************

debug_char:
	push	TMP
	ldi		TMP, '{'
	sts		asciicode, TMP
	rcall	set_output
	pop		TMP
	sts		asciicode, TMP
	rcall	set_output
	ldi		TMP, '}'
	sts		asciicode, TMP
	rcall	set_output
	ret


debug_fire_IRQ:
	; Send IRQ pulse
	cbi 	IRQ_OUT_PORT, IRQ_OUT_PIN			; Set IRQ pin low
	ldi		Fparam1, high(IRQ_PULSE_DELAY_US)	; Delay value high byte
	ldi		Fparam0, low(IRQ_PULSE_DELAY_US)	; Delay value low byte
	call	delay_ms
	sbi 	IRQ_OUT_PORT, IRQ_OUT_PIN			; Set IRQ pin high
	ret

; ------------------------------------------------------------------------------
; -----   DELAY_1MS                                                         -----
; ------------------------------------------------------------------------------
; Assumes that the number of repetitions required is in TMP (r16)
; The middle loop produces a 1ms delay
delay_1ms:
	push	r22
	push	r23
	push	r24
	push	r25
	ldi		r23, 0
delay_1ms_loop_outer:
	ldi		r24, 0
delay_1ms_loop_middle:
	ldi		r25, 0
delay_1ms_loop_inner:
	ldi		r22, 0
delay_1ms_loop_delay:
	nop
	nop
	nop
	nop
	inc		r22
	cpi		r22, 81
	brne	delay_1ms_loop_delay

	inc		r25
	cpi		r25, 24
	brne	delay_1ms_loop_inner

	inc		r24
	cpi		r24, 255
	brne	delay_1ms_loop_middle

	inc		r23
	cp		r23, TMP
	brsh	delay_1ms_loop_outer
	pop		r25
	pop		r24
	pop		r23
	pop		r22
	ret

; ------------------------------------------------------------------------------
; -----   DELAY_10US                                                       -----
; ------------------------------------------------------------------------------
; Assumes that the number of repetitions required is in TMP (r16)
; The inner loop produces a 10µs delay.
delay_10us:
	push	r24
	push	r25
	ldi		r25, 0
delay_10us_loop_outer:
	ldi		r24, 0
delay_10us_loop_inner:				; Inner loop takes 10µs
	inc		r24
	cpi		r24, 37
	brne	delay_10us_loop_inner
	inc		r25
	cp		r25, TMP
	brsh	delay_10us_loop_outer
	pop		r25
	pop		r24
	ret
