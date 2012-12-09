###############################################################################
#
# HFS/2 - A Hierarchical File System Driver for OS/2
# Copyright (C) 1996, 1997 Marcus Better
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
###############################################################################

#****************************************************************************
# Makefile for HFS/2 Filter Driver.
#****************************************************************************

.AUTODEPEND

#****************************************************************************
#  Environment Setup for the component(s).
#****************************************************************************

NAME = hfsflt
FILTER = $(NAME).flt

# Device Driver Kit path
DDKPATH = \PROG\DDK

#
# Compiler Location w/ includes, libs and tools
#

DK_TOOLS	= $(DDKPATH)\tools
INC		= $(DDKPATH)\base\inc
H      		= $(DDKPATH)\base\h
LIB		= $(DDKPATH)\base\lib
TOOLSPATH 	= $(DDKPATH)\base\tools
DISKH		= $(DDKPATH)\base\src\dev\dasd\diskh
DHLIB		= $(DDKPATH)\base\src\dev\dasd\devhelp
ADDLIB		= $(DDKPATH)\base\src\dev\dasd\addcalls
BORLANDC	= \prog\borlandc\bin
BCHEADERS       = \prog\borlandc\include
HFLAGS		= -d -f -w -s

# Set environment variables for compiler/linker and tools.
# NMAKE only.
#!if [set INCLUDE=$(H);$(INC)] || \
#   [set LIB=$(LIB);$(DHLIB);$(ADDLIB)] || \
#   [set PATH=$(TOOLSPATH);$(DK_TOOLS);$(BORLANDC)]
#!endif


#
# Compiler/tools Macros
#

AS=tasm
CC=bcc
IMPLIB=implib
IPF=ipfc
LIBUTIL=lib
LINK=link
MAPSYM=mapsym
RC=rc

#
# Compiler and Linker Options
#

# For MASM
#AFLAGS = -MX -T -Z $(ENV)
AFLAGS = /mx /zi /o
AINC   = -I. -I$(INC)
CINC   = -I$(H) -I$(DISKH) -I$(BCHEADERS)
# For MSC6
#CFLAGS = /c /Zp /G2s /Answ /W3 $(ENV)
#DFLAGS = /c /Zp /G2s /Answ /W3 $(ENV) -DDEBUG
# For Borland C++ 3.1
#CFLAGS = -DDEBUG
CFLAGS =
#LFLAGS = /map /nod /exepack /packd /a:16 /far
LFLAGS = /code /line /map
RFLAGS = -r

LIBS	=  $(DHLIB)\dhcalls.lib $(ADDLIB)\addcalls.lib
DEFFILE =  $(NAME).def
CFGFILE =  $(NAME).cfg

#****************************************************************************
#
# OBJ files
# The order of these is important!
OBJ1 =  segs.obj boot.obj strat1.obj filter.obj io.obj globals.obj
OBJ2 =  init.obj initutil.obj
OBJ3 =
#
#       Debug files
#
DBJ1 =  segs.obj boot.obj strat1.obj filter.obj io.obj
DBJ2 =  vsprintf.obj log.obj globals.obj
DBJ3 =  init.obj initutil.obj
#
#       LIST Files
#
LST1=
LST2=

LIST=  $(LST1) $(LST2) $(LST3)
OBJS = $(OBJ1) $(OBJ2) $(OBJ3)
DBJS = $(DBJ1) $(DBJ2) $(DBJ3)

#****************************************************************************
# Implicit rules
#****************************************************************************

.asm.obj:
        $(AS) $(AFLAGS) $(AINC) $*.asm;

.asm.mbj:
        $(AS) $(AFLAGS) -DMMIOPH $(AINC) $*.asm $*.mbj;

.asm.lst:
        $(AS) -l -n $(AFLAGS) $(AINC) $*.asm;

.c.obj:
        $(CC) $(CFLAGS) +$(CFGFILE) $*.c

.c.dbj:
        $(CC) $(DFLAGS) $(CINC) $*.c

.c.lst:
        $(CC) $(CFLAGS) /Fc $(CINC) $*.c
        copy $*.cod $*.lst
        del $*.cod

.h.inc:
        H2INC $(HFLAGS) $*.h -o $*.inc


#****************************************************************************
# Targets
#****************************************************************************

install: $(FILTER)
	copy $(FILTER) e:\os2\boot
	copy $(NAME).sym e:\os2\boot

all:   $(FILTER)

inc:

list: $(LIST)

clean:
        if exist *.lnk  del *.lnk
        if exist *.obj  del *.obj
        if exist *.map  del *.map
        if exist *.lst  del *.lst
        if exist *.sym  del *.sym
	if exist *.flt  del *.flt

#*****************************************************************************

$(FILTER):  $(CFGFILE) $(OBJS) $(LIBS) $(DEFFILE) makefile
        $(LINK) $(LFLAGS) @&&|
$(OBJS)
$(FILTER)
$*.map
$(LIBS)
$(DEFFILE)
|
        $(MAPSYM) $*.map
	tdstrip $(FILTER)

$(CFGFILE): makefile
	copy &&|
-Tla
-Tt
-B
#-S
-c
-3
-O2
-d
-ms!
-wpro
-weas
-wpre
-w-stu
$(CINC)
| $(CFGFILE)


#****************************************************************************
