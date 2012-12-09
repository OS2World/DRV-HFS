;
; HFS/2 - A Hierarchical File System Driver for OS/2
; Copyright (C) 1996, 1997 Marcus Better
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;

;;; segs.asm
;;; Segment declarations and ordering

	include devhdr.inc

DDHeader        segment dword public 'DATA'

DiskDDHeader   dd      -1
               dw      DEVLEV_3 + DEV_CHAR_DEV
               dw      _Strat1
               dw      0
               db      'HFSFLT$ '
               dw      0
               dw      0
               dw      0
               dw      0
               dd      DEV_ADAPTER_DD
               dw      0
DDHeader        ends

LIBDATA         segment dword public 'DATA'
LIBDATA         ends

_DATA           segment dword public 'DATA'
_DATA           ends

_BSS            segment dword public 'BSS'
_BSS            ends

_TEXT           segment dword public 'CODE'

                extrn  _Strat1:near

_TEXT           ends

Code            segment dword public 'CODE'
Code            ends

LIBCODE         segment dword public 'CODE'
LIBCODE         ends

DGROUP          group   _BSS, DDHeader, LIBDATA, _DATA
StaticGroup     group   Code, LIBCODE, _TEXT

        end
