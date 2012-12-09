#
# Rules.make
# This file contains rules which are shared between multiple Makefiles.
#

#
# False targets.
#
.PHONY: dummy 
.PHONY: first_rule sub_dirs all_targets dep

#
# Special variables which should not be exported
#
unexport EXTRA_ASFLAGS
unexport EXTRA_CFLAGS
unexport EXTRA_LDFLAGS
unexport EXTRA_ARFLAGS
unexport SUBDIRS
unexport SUB_DIRS
unexport ALL_SUB_DIRS
unexport O_TARGET
unexport O_OBJS
unexport L_OBJS
# objects that export symbol tables
unexport OX_OBJS
unexport LX_OBJS
unexport SYMTAB_OBJS

#
# Get things started.
#
first_rule: sub_dirs
	$(MAKE) all_targets

#
# Common rules
#

%.s: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -S $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

%.obj: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -Zomf -c -o $@ $<

%.o: %.s
	$(AS) $(ASFLAGS) $(EXTRA_ASFLAGS) -o $@ $<

%.lib: %.a
	$(EMXOMF) $(EMXOMF_FLAGS) -o $@ $<

%.exe: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $^

#
#
#
all_targets: $(O_TARGET) $(L_TARGET)

#
# Rule to compile a set of .o files into one .o file
#
ifdef O_TARGET
ALL_O = $(OX_OBJS) $(O_OBJS)
$(O_TARGET): $(ALL_O)
	if exist $@ del $@
ifneq "$(strip $(ALL_O))" ""
	$(LD) $(EXTRA_LDFLAGS) -r -o $@ $(ALL_O)
else
	$(AR) rcs $@
endif
endif

#
# Rule to compile a set of .o files into one .a file
#
ifdef L_TARGET
$(L_TARGET): $(LX_OBJS) $(L_OBJS)
	if exist $@ del $@
	$(AR) $(EXTRA_ARFLAGS) rcs $@ $(LX_OBJS) $(L_OBJS)
endif

#
# Make dependencies
#
dep: dummy
	if exist *.c gcc -M -I$(CINCL_DEP) *.c > .depend
ifdef ALL_SUB_DIRS
	for %i in ($(ALL_SUB_DIRS)) do $(MAKE) -C %i dep
endif

#.depend: dummy
#	gcc -M -I$(CINCL_DEP) *.c > .depend

#
# A rule to make subdirectories
#
sub_dirs: dummy
ifdef SUB_DIRS
	for %i in ($(SUB_DIRS)) do $(MAKE) -C %i
endif

#
# A rule to do nothing
#
dummy:

#
# include dependency file if it exists
#
include .depend
