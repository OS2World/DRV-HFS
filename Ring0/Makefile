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

# Makefile for ring 0 piece

.AUTODEPEND

# Project constants
NAME       = hfs
IFS        = $(NAME).ifs
MAP        = $(NAME).map
DEF        = $(NAME).def
CFG        = $(NAME).cfg
CSM        = $(NAME).csm

# Pathnames
DDKPATH    = \prog\ddk
DDKHEADERS = $(DDKPATH)\base\h
DDKINC     = $(DDKPATH)\base\inc
DDKLIB     = $(DDKPATH)\base\lib
DHLIB      = $(DDKPATH)\base\src\dev\dasd\devhelp
IFSTOOLKIT = \prog\ifstool
BORLANDC   = \prog\borlandc
BCLIB      = $(BORLANDC)\lib
BCHEADERS  = $(BORLANDC)\include

# Translator Definitions
#CFLAGS     = -DDEBUG
CFLAGS      =
CC         = bcc $(CFLAGS) +$(CFG)
TASM       = TASM
TLIB       = tlib
TLINK      = tlink
LINK       = link

TLINKOPT   = /3/n/e/s/Tod
LFLAGS     = /code /line /map
CINCL      = $(BCHEADERS);$(DDKHEADERS);..

# Implicit Rules
.c.obj:
  $(CC) -c {$< }

# Targets
OBJS = c0.obj \
 stubs.obj \
 r0comm.obj \
 r0global.obj \
 log.obj \
 vsprintf.obj
# vsprintf must be provided if DEBUG is defined, otherwise not.

LIBS = $(IFSTOOLKIT)\fshelper.lib $(DHLIB)\dhcalls.lib $(BCLIB)\cl.lib \
       $(DDKLIB)\os2286.lib

$(IFS): $(OBJS) $(DEF) $(CFG)
  $(LINK) $(LFLAGS) @&&|
$(OBJS)
$(IFS)
$(MAP)
$(LIBS)
$(DEF)
|

#$(IFS): $(OBJS) $(DEF) $(CFG)
#  $(TLINK) $(TLINKOPT) /L$(LIBPATH) @&&|
#$(OBJS)
#$(IFS)
#$(MAP)
#$(LIBS)
#$(DEF)
#|

# Individual File Dependencies
stubs.obj: $(CFG) stubs.c

r0comm.obj: $(CFG) r0comm.c

r0global.obj: $(CFG) r0global.c

vsprintf.obj: $(CFG) vsprintf.c

log.obj: $(CFG) log.c

c0.obj: $(CFG) c0.asm
	$(TASM) /d__LARGE__ /MX /i$(AINCL) c0.ASM,c0.OBJ

# Compiler Configuration File
$(CFG): Makefile
  copy &&|
#-S
-C      # Nested comments
-K
-N-
-a-     # Align on byte
-ml!    # Model large, DS!=SS
-3      # 386 instructions
-f-
-G
-O
-Og
-Oe
-Om
-Ov
-Ol
-Op
-O-i
-Z
-k-
-d
-vi-
-H=$(CSM)
-weas
-wpre
-I$(CINCL)
-L$(LIBPATH)
#-P.C
| $(CFG)