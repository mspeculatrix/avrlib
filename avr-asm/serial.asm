; ******************************************************************************
; *****   SERIAL.ASM                                                       *****
; ******************************************************************************
;
; REQUIREMENTS:
;
; To use this library, the following registers need to have been defined:
;	Fparam0
;	Fparam1
;	TMP
;	DataReg
;
; To use Serial_send_outbuf, you need to have a buffer labelled serial_out_buf
; in SRAM.
;
; You also need to have configured the UART/USART.
; This is a sample configuration for 9600 baud 8N1 for an Atmega328 at 16MHz.
	; ldi		TMP, (1 << U2X0)		; Double speed & clear existing transmits
	; sts		UCSR0A, TMP

	; ldi		TMP, (1 << RXEN0) | (1 << TXEN0)	; enable TX and RX
	; sts		UCSR0B, TMP

	; ldi		TMP, (1 << UPM01) | (1 << UCSZ01) | (1 << UCSZ00); 8 bit, even parity
	; sts		UCSR0C, TMP

	; ldi		TMP, high(207)			; Set baud rate
	; sts		UBRR0H, TMP
	; ldi		TMP, low(207)
	; sts		UBRR0L, TMP

	; lds		TMP, UCSR0A
	; ori		TMP, (1 << TXC0)			; clear any existing transmits
	; sts		UCSR0A, TMP

; ******************************************************************************
; *****   MACROS                                                          *****
; ******************************************************************************

.MACRO SERIAL_SEND_STR
	ldi		Fparam0, low(@0 << 1)
	ldi		Fparam1, high(@0 << 1)
	rcall	Serial_send_string
.ENDMACRO



; ******************************************************************************
; *****   SENDING                                                          *****
; ******************************************************************************


; ------------------------------------------------------------------------------
; -----   SERIAL_SEND_ESTR                                                 -----
; ------------------------------------------------------------------------------
; To send a string from EEPROM.
; Assumes the address of the string is in Fparam0 (low) and Fparam1 (high)
Serial_send_estr:
	out		EEARH, Fparam1	; address registers.
	out		EEARL, Fparam0
	sbi		EECR, EERE				; Inititate EEPROM byte read at address
	in		DataReg, EEDR			; Read pin value from EEPROM data register
	cpi		DataReg, 0
	breq	Serial_send_estr_done
	rcall	Serial_send_char
	inc		Fparam0
	cpi		Fparam0, 0				; If it's 0, it has rolled over
	brne	Serial_send_estr
	inc		Fparam1
	rjmp	Serial_send_estr
Serial_send_estr_done:
	ret


; ------------------------------------------------------------------------------
; -----   SERIAL_SEND_OUTBUF                                               -----
; ------------------------------------------------------------------------------
; Send the contents of out_buf in RAM.
Serial_send_outbuf:
	ldi		ZH, high(serial_out_buf)
	ldi		ZL, low(serial_out_buf)
Serial_send_outbuf_loop:
	ld		DataReg, Z+
	cpi		DataReg, 0					; Is it the null terminator?
	breq	Serial_send_outbuf_done
	rcall	Serial_send_char
	rjmp 	Serial_send_outbuf_loop
Serial_send_outbuf_done:
	ret

; ------------------------------------------------------------------------------
; -----   SERIAL_SEND_STRING                                               -----
; ------------------------------------------------------------------------------
; To send a string from program memory.
; Assumes the address of the string is in Fparam0 (low) and Fparam1 (high).
Serial_send_string:
	lsl		Fparam0
	lsl		Fparam1
	mov		ZH, Fparam1			; Load Z indirection vector with address of
	mov		ZL, Fparam0			; the string.
Serial_send_str_loop:
	lpm		DataReg, Z+			; Get next char & increment indirection vector
	cpi		DataReg, 0			; Is it the null terminator?
	breq	Serial_send_str_done	; If so, we're at the end of the string
	rcall	Serial_send_char		; Otherwise, send the character
	rjmp 	Serial_send_str_loop
Serial_send_str_done:
	ret

; ------------------------------------------------------------------------------
; -----   SERIAL_SEND_CHAR                                                 -----
; ------------------------------------------------------------------------------
; Assumes char to be sent is in DataReg.
Serial_send_char:
	lds		TMP, UCSR0A					; Load control/status register A
	sbrs	TMP, UDRE0					; Check if Data Register Empty flag set
	rjmp	_send_char					; If zero, TX sent bit not set
	ori		TMP, (1 << TXC0)
	sts		UCSR0A, TMP
	sts		UDR0, DataReg				; Write to data reg to send
	ret
