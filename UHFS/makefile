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
# Makefile for HFS/2 File System Utilities DLL
#****************************************************************************

.AUTODEPEND

#****************************************************************************
#  Environment Setup for the component(s).
#****************************************************************************

NAME = uhfs
DLL  = $(NAME).dll

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
BCLIB           = \prog\borlandc\lib
HFLAGS		= -d -f -w -s


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
TLINK=tlink

#
# Compiler and Linker Options
#

AINC   = -I. -I$(INC)
CINC   = -I$(H) -I$(BCHEADERS)

# For MASM
#AFLAGS = -MX -T -Z $(ENV)

# For Turbo Assembler
AFLAGS = /d__LARGE__ /mx /zi /la

# For MSC6
#CFLAGS = /c /Zp /G2s /Answ /W3 $(ENV)
#DFLAGS = /c /Zp /G2s /Answ /W3 $(ENV) -DDEBUG

# For Borland C++ 3.1
#CFLAGS = -DDEBUG
CFLAGS =

# For TLINK
TLINKOPT = /n/e/s/c/Tod

# For LINK
#LFLAGS = /map /nod /exepack /packd /a:16 /far
LFLAGS = /code /line /map
RFLAGS = -r

LIBHFS_DIR  = libhfs
LIBHFS  = $(LIBHFS_DIR)\libhfs.lib
LIBS	= $(BCLIB)\cl.lib $(LIB)\os2286.lib $(LIBHFS)
DEFFILE =  $(NAME).def
CFGFILE =  $(NAME).cfg

#****************************************************************************
#
# OBJ files
#

#LIBHFS_OBJS = $(LIBHFS_DIR)\block.obj $(LIBHFS_DIR)\btree.obj \
#              $(LIBHFS_DIR)\data.obj $(LIBHFS_DIR)\file.obj   \
#              $(LIBHFS_DIR)\hfs.obj $(LIBHFS_DIR)\low.obj     \
#              $(LIBHFS_DIR)\node.obj $(LIBHFS_DIR)\record.obj \
#              $(LIBHFS_DIR)\volume.obj

OBJS = c0.obj chkdsk.obj format.obj recover.obj sys.obj misc.obj

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

all:   $(DLL)

clean:
        if exist *.lnk  del *.lnk
        if exist *.obj  del *.obj
        if exist *.map  del *.map
        if exist *.lst  del *.lst
        if exist *.sym  del *.sym
	if exist *.dll  del *.dll

#*****************************************************************************

#$(DLL): $(CFGFILE) $(OBJS) $(LIBS) $(DEFFILE) makefile
#  $(TLINK) $(TLINKOPT) /L$(LIBPATH) @&&|
#$(OBJS)
#$(DLL),$(MAP)
#$(LIBS)
#$(DEFFILE)
#|

$(DLL):  $(CFGFILE) $(OBJS) $(LIBS) $(DEFFILE) makefile
        $(LINK) $(LFLAGS) @&&|
$(OBJS)
$(DLL)
$*.map
$(LIBS)
$(DEFFILE)
|

$(LIBHFS):
	cd $(LIBHFS_DIR)
	make
	cd ..

$(CFGFILE): makefile
	copy &&|
-a-
-Tla
-Tt
-B
-S
-c
-3
-O2
-d
-ml
-wpro
-weas
-wpre
-w-stu
$(CINC)
| $(CFGFILE)


#****************************************************************************
