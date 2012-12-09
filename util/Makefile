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

export
.PHONY: all subdirs clean

TOPDIR  =/prog/hfs/util

CC      =gcc
AS	=as
LD	=ld
CC	=gcc
AR	=ar
EMXOMF  =emxomf

EMXOMF_FLAGS = -q

all: programs subdirs

# If uncommented, create an .EXE file that doesn't require emx.dll.
# If commented out, create a.out style output which can be debugged by gdb.
OMF = YES

#
# standard CFLAGS
#

# These are for debugging
#CFLAGS = -g -Zmt -DDEBUG -Wall
#LDFLAGS = 

# These are for release.
CFLAGS = -Zmt -Wall -O
LDFLAGS = -s

# Include path for make dep - must not contain any colons
CINCL_DEP = /prog/emx/include

# The object and library/archive extensions
ifdef OMF
OBJ_EXT = obj
LIB_EXT = lib
else
OBJ_EXT = o
LIB_EXT = a
endif

ifdef OMF
LDFLAGS += -Zsys -Zomf
endif

SUBDIRS		=Prepare
ALL_SUB_DIRS	=$(SUBDIRS)

FSKILL =  fskill.exe
SYNC =    sync.exe
READRES = readres.exe
BINDISK = bindisk.exe

programs: $(FSKILL) $(SYNC) $(READRES) $(BINDISK)

subdirs: dummy
	$(MAKE) -C Prepare

clean: dummy
	if exist *.exe del *.exe
	if exist *.obj del *.obj
	if exist *.o del *.o
	if exist *~ del *~
	if exist core del core
	$(MAKE) -C Prepare clean

include Rules.make
