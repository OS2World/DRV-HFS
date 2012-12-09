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
unexport NAME
unexport EXE_NAME
unexport DEF_NAME
unexport RING3_OBJS
.PHONY: all subdirs clean

TOPDIR  =/prog/hfs/ring3

CC      =gcc
AS	=as
LD	=ld
CC	=gcc
AR	=ar
EMXOMF  =emxomf

EMXOMF_FLAGS = -q

# Project info
NAME    =hfs
EXE_NAME=$(NAME).exe
DEF_NAME=$(NAME).def 

all: $(EXE_NAME)

# If uncommented, create an .EXE file that doesn't require emx.dll.
# If commented out, create a.out output which can be debugged by gdb.
OMF = YES

#
# standard CFLAGS
#

# These are for debugging
#CFLAGS = -g -Zmt -DDEBUG -D__OS2__ -DIFS -DNODEVLOCKS -Wall
#LDFLAGS = 

# These are for release. Optimizations (-O or -O2) seem to break something.(??)
CFLAGS = -Zmt -D__OS2__ -DIFS -DNODEVLOCKS -DINCL_32 -Wall
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

#
ARCHIVES	=libhfs/libhfs.$(LIB_EXT)
SUBDIRS		=libhfs
ALL_SUB_DIRS	=$(SUBDIRS)

# The ring 3 modules
RING3_MODULES = r3comm logger log diskfunc dir dirsearch cache userfunc
RING3_MODULES += vol file misc names times fsctl error ea attr resource
RING3_MODULES += icon colour
RING3_OBJS = $(addsuffix .$(OBJ_EXT),$(RING3_MODULES))

$(EXE_NAME): $(RING3_OBJS) subdirs $(ARCHIVES)
	$(CC) -o $(EXE_NAME) $(CFLAGS) $(LDFLAGS) \
	  $(DEF_NAME) \
	  $(RING3_OBJS) \
	  $(ARCHIVES)

subdirs: dummy
	$(MAKE) -C libhfs

clean: dummy
	if exist $(EXE_NAME) del $(EXE_NAME)
	if exist *.obj del *.obj
	if exist *.o del *.o
	if exist *~ del *~
	if exist core del core
	$(MAKE) -C libhfs clean

include Rules.make
