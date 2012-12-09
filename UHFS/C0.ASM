;[]------------------------------------------------------------[]
;|      C0.ASM -- Start Up Code for DOS                         |
;[]------------------------------------------------------------[]

;
;       C/C++ Run Time Library - Version 5.0
;
;       Copyright (c) 1987, 1992 by Borland International
;       All Rights Reserved.
;

                locals

;       Segment and Group declarations

_TEXT           SEGMENT BYTE PUBLIC 'CODE'
                ENDS
_FARDATA        SEGMENT PARA PUBLIC 'FAR_DATA'
                ENDS
_FARBSS         SEGMENT PARA PUBLIC 'FAR_BSS'
                ENDS
_DATA           SEGMENT PARA PUBLIC 'DATA'
                ENDS
_CONST          SEGMENT WORD PUBLIC 'CONST'
                ENDS
_CVTSEG         SEGMENT WORD PUBLIC 'DATA'
                ENDS
_SCNSEG         SEGMENT WORD PUBLIC 'DATA'
                ENDS
_INIT_          SEGMENT WORD PUBLIC 'INITDATA'
InitStart       label byte
                ENDS
_INITEND_       SEGMENT BYTE PUBLIC 'INITDATA'
InitEnd         label byte
                ENDS
_EXIT_          SEGMENT WORD PUBLIC 'EXITDATA'
ExitStart       label byte
                ENDS
_EXITEND_       SEGMENT BYTE PUBLIC 'EXITDATA'
ExitEnd         label byte
                ENDS

IFNDEF __HUGE__
_BSS          SEGMENT WORD PUBLIC 'BSS'
bdata@        label byte
              ENDS
_BSSEND       SEGMENT BYTE PUBLIC 'BSSEND'
edata@        label byte
              ENDS
ENDIF


IFDEF   __SMALL__ OR __MEDIUM__ OR __COMPACT__ OR __LARGE__
      DGROUP GROUP _DATA,_CONST,_CVTSEG,_SCNSEG,_INIT_,_INITEND_,_EXIT_,_EXITEND_,_BSS,_BSSEND
ELSEIFDEF __HUGE__
      DGROUP GROUP _DATA,_CONST,_CVTSEG,_SCNSEG,_INIT_,_INITEND_,_EXIT_,_EXITEND_
ELSE
      %OUT  You must supply a model symbol.
      .ERR
ENDIF


;/*                                                     */
;/*-----------------------------------------------------*/
;/*                                                     */
;/*     Start Up Code                                   */
;/*     -------------                                   */
;/*                                                     */
;/*-----------------------------------------------------*/
;/*                                                     */

;
;       At the start, DS points to DGROUP
;

                ASSUME  CS:_TEXT, DS:DGROUP

_TEXT           SEGMENT
                PUBLIC  STARTUP
STARTUP         PROC    FAR

                push    ds
                pop     es

IFNDEF  __HUGE__

;       Reset uninitialized data area

                xor     ax, ax
                mov     di, offset DGROUP: bdata@
                mov     cx, offset DGROUP: edata@
                sub     cx, di
                cld
                rep     stosb
ENDIF

                mov     si,offset DGROUP:InitStart      ;si = start of table
                mov     di,offset DGROUP:InitEnd        ;di = end of table

;------------------------------------------------------------------
;  Loop through a startup/exit (SE) table,
;  calling functions in order of priority.
;  ES:SI is assumed to point to the beginning of the SE table
;  ES:DI is assumed to point to the end of the SE table
;  First 64 priorities are reserved by Borland
;------------------------------------------------------------------
PNEAR           EQU     0
PFAR            EQU     1
NOTUSED         EQU     0ffh

SE              STRUC
calltype        db      ?                       ; 0=near,1=far,ff=not used
priority        db      ?                       ; 0=highest,ff=lowest
addrlow         dw      ?
addrhigh        dw      ?
SE              ENDS

@@Start:        mov     ax,100h                 ;start with lowest priority
                mov     dx,di                   ;set sentinel to end of table
                mov     bx,si                   ;bx = start of table

@@TopOfTable:   cmp     bx,di                   ;and the end of the table?
                je      @@EndOfTable            ;yes, exit the loop
                cmp     es:[bx.calltype],NOTUSED;check the call type
                je      @@Next
                mov     cl, es:[bx.priority]    ;move priority to CX
                xor     ch, ch
                cmp     cx,ax                   ;check the priority
                jae     @@Next                  ;too high?  skip
                mov     ax,cx                   ;keep priority
                mov     dx,bx                   ;keep index in dx
@@Next:         add     bx,SIZE SE              ;bx = next item in table
                jmp     @@TopOfTable

@@EndOfTable:   cmp     dx,di                   ;did we exhaust the table?
                je      @@Done                  ;yes, quit
                mov     bx,dx                   ;bx = highest priority item
                cmp     es:[bx.calltype],PNEAR  ;is it near or far?
                mov     es:[bx.calltype],NOTUSED;wipe the call type
                push    es                      ;save es
                je      @@NearCall

@@FarCall:      call    DWORD PTR es:[bx.addrlow]
                pop     es                      ;restore es
                jmp     short @@Start

@@NearCall:     call    WORD PTR es:[bx.addrlow]
                pop     es                      ;restore es
                jmp     short @@Start

@@Done:         ret

STARTUP         ENDP


;------------------------------------------------------------------

                public _abort
_abort           label   word
;
                public __RealCvtVector
__RealCvtVector label word
;
                public _ScanTodVector
_ScanTodVector label word
;

_TEXT           ENDS

_DATA           SEGMENT
                db      'Borland C++ - Copyright 1991 Borland Intl.',0
                public _errno
_errno          label word
		public _environ
_environ	dd	0
;		public _psp
;_psp		dw	0
;		public _heapbase
;_heapbase	dd	0
;		public _heaptop
;_heaptop	dd	0
;		public _brklvl
;_brklvl		dd	0
;
_DATA           ENDS

                END
