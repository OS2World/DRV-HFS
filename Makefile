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

#**************************************************************************
# Top-level makefile for HFS/2
#**************************************************************************

#**************************************************************************
# Path names
#**************************************************************************
DDKPATH              = /prog/ddk

#**************************************************************************
# Executables
#**************************************************************************
ZIP                  = /tools/bin/zip
IPFC                 = $(DDKPATH)/tools/ipfc

#**************************************************************************
# Target file names
#**************************************************************************

NAME                 = hfs

ARCHIVE_EXT          = zip
DIST_ARCHIVE         = $(NAME)$(VER).$(ARCHIVE_EXT)
SRC_ARCHIVE          = $(NAME)src.$(ARCHIVE_EXT)

DOCPATH              = doc
IPFFILE              = $(DOCPATH)/$(NAME).ipf
INFFILE              = $(DOCPATH)/$(NAME).inf

#**************************************************************************
# Version information
#**************************************************************************
VER_MAJOR = 0
VER_MINOR = 10
VER       = $(VER_MAJOR)$(VER_MINOR)

#**************************************************************************
# File list
#**************************************************************************
include files.lst

#**************************************************************************
# Rules
#**************************************************************************

dist: $(DIST_ARCHIVE)

doc: $(INFFILE)

$(DIST_ARCHIVE): $(DIST_FILES)
	$(ZIP) -j $@ $(DIST_FILES)

$(SRC_ARCHIVE): $(SRCFILES)
	$(ZIP) $@ $(SRCFILES)

$(INFFILE): $(IPFFILE)
	$(MAKE) -C $(DOCPATH)
