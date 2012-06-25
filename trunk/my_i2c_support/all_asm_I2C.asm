 .cdecls C,LIST,
 %{
	#include "msp430x20x3.h"
	#define      RXTXI2C  R7
	#define      ADDRI2C  R8
	#define      DATAI2C  R9
	#define      BITI2C   R10
%}
;            Definitions for I2C bus
TMPADDR      .equ   090h                     ; TMP100 HW Address (A0=A1=0+WR)
SDA          .equ   001h                     ; P2.0 controls SDA line (pull-up)
SCL          .equ   002h                     ; P2.1 controls SCL line (pull-up)
N_BITS		 .equ   008h

;------------------------------------------------------------------------------
            .text
;------------------------------------------------------------------------------
;///////////I2C Subroutines start//////////////////////////////////////////////
;------------------------------------------------------------------------------
 .global Read_I2C   ; Reads  bytes of data transmitted from slave
;            enter ADDRI2C=00 - FF I2C device address to read
;                  RXTXI2C=x
;                  DATAI2C=x
;            exit  ADDRI2C=x
;                  RXTXI2C=x
;                  DATAI2C=0000 - FFFF I2C device data
;------------------------------------------------------------------------------
            mov.b   #TMPADDR,RXTXI2C        ; Load HW Address
            call    #I2C_Start              ; Send Start, Address and Ack
            mov.b   ADDRI2C,RXTXI2C         ; Load Pointer Address
            call    #I2C_TX                 ; Send Pointer and Ack
            mov.b   #TMPADDR,RXTXI2C        ; Load HW Address
            bis.b   #01h,RXTXI2C            ; Set LSB for "READ"
            call    #I2C_Start              ; Send Start, Address+RD and Ack
            call    #I2C_RX_Byte            ; Read Data and Ack
                                            ;
;***** Used for 2-Byte transfer only *****  ;
;            call    #I2C_ACKn               ; Acknowledge Byte Rcv'd
;            call    #I2C_RX_Byte                 ; Read Data and Ack
                                            ;
            call    #I2C_NACKn              ; NOT Acknowledge Byte Rcv'd
            call    #I2C_Stop               ; Send Stop
            ret                             ; Return from subroutine
                                            ;
;------------------------------------------------------------------------------
 .global Write_I2C;  enter ADDRI2C=00 - FF I2C device address to write to
;                 RXTXI2C=x
;                 DATAI2C=00 - FF I2C device data to write
;           exit  ADDRI2C=x
;                 RXTXI2C=x
;                 DATAI2C=x
;------------------------------------------------------------------------------
            mov.b   #TMPADDR,RXTXI2C        ; Load HW Address
            call    #I2C_Start              ; Send Start, Address and Ack
            mov.b   ADDRI2C,RXTXI2C         ; Load Pointer Address
            call    #I2C_TX                 ; Send Pointer and Ack
            mov.b   DATAI2C,RXTXI2C         ; Load Out-Going Data
            call    #I2C_TX                 ; Send Data and Ack
            call    #I2C_Stop               ; Send Stop
            ret                             ; Return from subroutine
                                            ;
;------------------------------------------------------------------------------
I2C_Start;  enter SDA=1, SCL=x
;           exit  SDA=1, SCL=0
;------------------------------------------------------------------------------
            bic.b   #SCL+SDA,&P2DIR         ; SCL and SDA to input direction
            bic.b   #SCL+SDA,&P2OUT         ; SCL=1, SDA=1
            bis.b   #SDA,&P2DIR             ; SDA=0
            bis.b   #SCL,&P2DIR             ; SCL=0
;------------------------------------------------------------------------------
I2C_TX;     enter SDA=x, SCL=0
;           exit  SDA=1, SCL=0
;------------------------------------------------------------------------------
            mov     #N_BITS,BITI2C          ; number of bits to xfer
I2C_TX_Bit  rla.b   RXTXI2C                 ; data bit -> carry
            jc      I2C_TX1                 ; test carry for 1 or 0
I2C_TX0     bis.b   #SDA,&P2DIR             ; SDA=0
            jmp     I2C_TXx                 ; Toggle SCL
I2C_TX1     bic.b   #SDA,&P2DIR             ; SDA=1
I2C_TXx     bic.b   #SCL,&P2DIR             ; SCL=1
            nop                             ; delay to meet I2C spec
            nop                             ;
            bis.b   #SCL,&P2DIR             ; SCL=0
            dec     BITI2C                  ; all bits read?
            jnz     I2C_TX_Bit              ; continue until 8 bits are sent
            bic.b   #SDA,&P2DIR             ; SDA=1
                                            ;
TX_Ackn     bic.b   #SCL,&P2DIR             ; SCL=1
            nop                             ; delay to meet I2C spec
            nop                             ;
            bis.b   #SCL,&P2DIR             ; SCL=0
                                            ;
            ret                             ; Return from subroutine
                                            ;
;------------------------------------------------------------------------------
I2C_RX_Byte  ;   enter SDA=1, SCL=0
;           exit  SDA=x, SCL=0
;------------------------------------------------------------------------------
            mov.b   #N_BITS,BITI2C          ; number of bits to rcv
I2C_RX_Bit  bic.b   #SCL,&P2DIR             ; SCL=1
            bit.b   #SDA,&P2IN              ; SDA bit -> carry
            rlc.w   DATAI2C                 ; Shift new bit into DATAI2C
            bis.b   #SCL,&P2DIR             ; SCL=0
            dec     BITI2C                  ; all bits read?
            jnz     I2C_RX_Bit              ; continue until 8 bits are read
                                            ;
            ret                             ; Return from subroutine
                                            ;
                                            ;
;------------------------------------------------------------------------------
I2C_ACKn;   enter SDA=x, SCL=0
;           exit  SDA=0, SCL=0
;------------------------------------------------------------------------------
            bis.b   #SDA,&P2DIR             ; SDA=0, Ack
            bic.b   #SCL,&P2DIR             ; SCL=1
            bis.b   #SCL,&P2DIR             ; SCL=0
            bic.b   #SDA,&P2DIR             ; SDA=1
Ackn_End    ret                             ; Return from subroutine
                                            ;
;------------------------------------------------------------------------------
I2C_NACKn;  enter SDA=x, SCL=0
;           exit  SDA=1, SCL=0
;------------------------------------------------------------------------------
            bic.b   #SDA,&P2DIR             ; SDA=1, NOT Ack
            bic.b   #SCL,&P2DIR             ; SCL=1
            bis.b   #SCL,&P2DIR             ; SCL=0
NAckn_End   ret                             ; Return from subroutine
                                            ;
;------------------------------------------------------------------------------
I2C_Stop;   enter SDA=x, SCL=0
;           exit  SDA=1, SCL=1
;------------------------------------------------------------------------------
            bis.b   #SDA,&P2DIR             ; SDA = 0
            bic.b   #SCL,&P2DIR             ; SCL = 1
            bic.b   #SDA,&P2DIR             ; SDA = 1
I2C_End     ret                             ; Return from subroutine
                                            ;
;///////////I2C Subroutines stop///////////////////////////////////////////////


            .end                             ;
