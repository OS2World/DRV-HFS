.**********************************************************************
.* hfs.ipf
.**********************************************************************

:userdoc.
:docprof toc=12.

.nameit symbol=HFS text='HFS/2'

:title.&HFS.

.**********************************************************************
:h1.Title page
.**********************************************************************

:font facename=Helv size=28x18.
.ce HFS/2
:font facename=default size=0x0.
.ce A Hierarchical File System Driver for OS/2
.ce Copyright (C) 1996, 1997 Marcus Better

.**********************************************************************
:h1 id=100.Introduction
.**********************************************************************

:p.  
&HFS. is a file system driver for OS/2 
that reads and writes
Macintosh disks. With &HFS., Macintosh-formatted media can be used
seamlessly in OS/2.  The program has been tested with 
1.44MB diskettes :link reftype=fn refid=diskette.(**):elink.
and CD-ROMs. Other removable media may also work, and
fixed-disk support will be added in future releases.

:hp8.
:warning.
This program is in an early, experimental stage, and may still have
bugs and problems.  
You should backup your system before using the program,
and make backup copies of any diskettes that you use with this program.
:ewarning.
:ehp8.
&HFS. is based on hfsutils, an implementation of the
Macintosh Hierarchical File System written by Robert Leslie.
I have found it helpful to 
look at the source code of Matthieu Willm's ext2-os2 driver and the
ext2flt partition filter written by Deon van der Westhuysen. The IFS part of
&HFS. follows an architecture for a split ring 0/ring 3 IFS described
in articles by Andre Asselin in the Electronic OS/2 Developer's
Magazine. Parts of &HFS. were developed using Eberhard Mattes' ports
of Emacs and gcc. 

:note.  
This version of HFS/2 uses a different HFS implementation than
previous versions. It may therefore have problems that were not
present in previous versions.

:p.
The latest version of &HFS. is available on the World Wide Web at 
:hp1.http&colon.//www.student.nada.kth.se/~f96-bet/HFS:ehp1.. This
page also contains bugfixes and updated versions of parts of the
package, including a debugging version of the control program that
prints information which is useful for locating problems.

:p.
I hope you will find &HFS. useful. Comments, success/bug reports,
patches and ideas are always welcome.

:p.
Marcus Better

:p.
:parml tsize=15 break=fit.
:pt.Email:
:pd.Marcus.Better@abc.se
:eparml.

.**********************************************************************
:h1 id=200.Copyright
.**********************************************************************
:sl compact.
:li.:link reftype=hd refid=210.Copyright notice for &HFS.:elink.
:li.:link reftype=hd refid=220.Copyright notice for hfsutils:elink.
:li.:link reftype=hd refid=230.The GNU General Public License:elink.
:esl.

:p.  
&HFS. also contains some code from the Linux kernel which is
copyrighted by Linus Torvalds and covered by the GPL.

:h2 id=210.Copyright notice for &HFS.

:p.
HFS/2 - A Hierarchical File System Driver for OS/2
.br
Copyright (C) 1996, 1997 Marcus Better
:p.
This program is free software; you can redistribute it and/or modify
it under the terms of the 
:link reftype=hd refid=230.GNU General Public License:elink.
as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
:p.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
:p.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

:h2 id=220.Copyright notice for hfsutils

:p.
hfsutils - tools for reading and writing Macintosh HFS volumes
.br
Copyright (C) 1996, 1997 Robert Leslie
:p.
This program is free software; you can redistribute it and/or modify
it under the terms of the 
:link reftype=hd refid=230.GNU General Public License:elink.
as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
:p.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
:p.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

:h2 id=230.The GNU General Public License

.*:font facename=Helv size=28x18.
:p.
.ce GNU GENERAL PUBLIC LICENSE
.ce Version 2, June 1991

:p.
:parml compact tsize=29 break=none.
:pt.
Copyright (C) 1989, 1991
:pd.
Free Software Foundation, Inc.
.br
675 Mass Ave, Cambridge, MA 02139, USA
:eparml.
Everyone is permitted to copy and distribute verbatim copies
of this license document, but changing it is not allowed.

.ce Preamble

:p.
The licenses for most software are designed to take away your
freedom to share and change it.  By contrast, the GNU General Public
License is intended to guarantee your freedom to share and change free
software--to make sure the software is free for all its users.  This
General Public License applies to most of the Free Software
Foundation's software and to any other program whose authors commit to
using it.  (Some other Free Software Foundation software is covered by
the GNU Library General Public License instead.)  You can apply it to
your programs, too.

:p.
When we speak of free software, we are referring to freedom, not
price.  Our General Public Licenses are designed to make sure that you
have the freedom to distribute copies of free software (and charge for
this service if you wish), that you receive source code or can get it
if you want it, that you can change the software or use pieces of it
in new free programs; and that you know you can do these things.

:p.
To protect your rights, we need to make restrictions that forbid
anyone to deny you these rights or to ask you to surrender the rights.
These restrictions translate to certain responsibilities for you if you
distribute copies of the software, or if you modify it.

:p.
For example, if you distribute copies of such a program, whether
gratis or for a fee, you must give the recipients all the rights that
you have.  You must make sure that they, too, receive or can get the
source code.  And you must show them these terms so they know their
rights.

:p.
We protect your rights with two steps: (1) copyright the software, and
(2) offer you this license which gives you legal permission to copy,
distribute and/or modify the software.

:p.
Also, for each author's protection and ours, we want to make certain
that everyone understands that there is no warranty for this free
software.  If the software is modified by someone else and passed on, we
want its recipients to know that what they have is not the original, so
that any problems introduced by others will not reflect on the original
authors' reputations.

:p.
Finally, any free program is threatened constantly by software
patents.  We wish to avoid the danger that redistributors of a free
program will individually obtain patent licenses, in effect making the
program proprietary.  To prevent this, we have made it clear that any
patent must be licensed for everyone's free use or not licensed at all.

:p.
The precise terms and conditions for copying, distribution and
modification follow.

:p.
.ce GNU GENERAL PUBLIC LICENSE
.ce TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

:p.
0. This License applies to any program or other work which contains
a notice placed by the copyright holder saying it may be distributed
under the terms of this General Public License.  The "Program", below,
refers to any such program or work, and a "work based on the Program"
means either the Program or any derivative work under copyright law:
that is to say, a work containing the Program or a portion of it,
either verbatim or with modifications and/or translated into another
language.  (Hereinafter, translation is included without limitation in
the term "modification".)  Each licensee is addressed as "you".

:p.
Activities other than copying, distribution and modification are not
covered by this License; they are outside its scope.  The act of
running the Program is not restricted, and the output from the Program
is covered only if its contents constitute a work based on the
Program (independent of having been made by running the Program).
Whether that is true depends on what the Program does.

:p.
1. You may copy and distribute verbatim copies of the Program's
source code as you receive it, in any medium, provided that you
conspicuously and appropriately publish on each copy an appropriate
copyright notice and disclaimer of warranty; keep intact all the
notices that refer to this License and to the absence of any warranty;
and give any other recipients of the Program a copy of this License
along with the Program.

:p.
You may charge a fee for the physical act of transferring a copy, and
you may at your option offer warranty protection in exchange for a fee.

:p.
2. You may modify your copy or copies of the Program or any portion
of it, thus forming a work based on the Program, and copy and
distribute such modifications or work under the terms of Section 1
above, provided that you also meet all of these conditions:

:lm margin=10.
:rm margin=10.
:p.
a) You must cause the modified files to carry prominent notices
stating that you changed the files and the date of any change.

:p.
b) You must cause any work that you distribute or publish, that in
whole or in part contains or is derived from the Program or any
part thereof, to be licensed as a whole at no charge to all third
parties under the terms of this License.

:p.
c) If the modified program normally reads commands interactively
when run, you must cause it, when started running for such
interactive use in the most ordinary way, to print or display an
announcement including an appropriate copyright notice and a
notice that there is no warranty (or else, saying that you provide
a warranty) and that users may redistribute the program under
these conditions, and telling the user how to view a copy of this
License.  (Exception: if the Program itself is interactive but
does not normally print such an announcement, your work based on
the Program is not required to print an announcement.)
:lm margin=1.
:rm margin=1.

:p.
These requirements apply to the modified work as a whole.  If
identifiable sections of that work are not derived from the Program,
and can be reasonably considered independent and separate works in
themselves, then this License, and its terms, do not apply to those
sections when you distribute them as separate works.  But when you
distribute the same sections as part of a whole which is a work based
on the Program, the distribution of the whole must be on the terms of
this License, whose permissions for other licensees extend to the
entire whole, and thus to each and every part regardless of who wrote it.

:p.
Thus, it is not the intent of this section to claim rights or contest
your rights to work written entirely by you; rather, the intent is to
exercise the right to control the distribution of derivative or
collective works based on the Program.

:p.
In addition, mere aggregation of another work not based on the Program
with the Program (or with a work based on the Program) on a volume of
a storage or distribution medium does not bring the other work under
the scope of this License.

:p.
3. You may copy and distribute the Program (or a work based on it,
under Section 2) in object code or executable form under the terms of
Sections 1 and 2 above provided that you also do one of the following:

:lm margin=10.
:rm margin=10.
:p.
a) Accompany it with the complete corresponding machine-readable
source code, which must be distributed under the terms of Sections
1 and 2 above on a medium customarily used for software interchange; or,

:p.
b) Accompany it with a written offer, valid for at least three
years, to give any third party, for a charge no more than your
cost of physically performing source distribution, a complete
machine-readable copy of the corresponding source code, to be
distributed under the terms of Sections 1 and 2 above on a medium
customarily used for software interchange; or,

:p.
c) Accompany it with the information you received as to the offer
to distribute corresponding source code.  (This alternative is
allowed only for noncommercial distribution and only if you
received the program in object code or executable form with such
an offer, in accord with Subsection b above.)
:lm margin=1.
:rm margin=1.

:p.
The source code for a work means the preferred form of the work for
making modifications to it.  For an executable work, complete source
code means all the source code for all modules it contains, plus any
associated interface definition files, plus the scripts used to
control compilation and installation of the executable.  However, as a
special exception, the source code distributed need not include
anything that is normally distributed (in either source or binary
form) with the major components (compiler, kernel, and so on) of the
operating system on which the executable runs, unless that component
itself accompanies the executable.

:p.
If distribution of executable or object code is made by offering
access to copy from a designated place, then offering equivalent
access to copy the source code from the same place counts as
distribution of the source code, even though third parties are not
compelled to copy the source along with the object code.

:p.
4. You may not copy, modify, sublicense, or distribute the Program
except as expressly provided under this License.  Any attempt
otherwise to copy, modify, sublicense or distribute the Program is
void, and will automatically terminate your rights under this License.
However, parties who have received copies, or rights, from you under
this License will not have their licenses terminated so long as such
parties remain in full compliance.

:p.
5. You are not required to accept this License, since you have not
signed it.  However, nothing else grants you permission to modify or
distribute the Program or its derivative works.  These actions are
prohibited by law if you do not accept this License.  Therefore, by
modifying or distributing the Program (or any work based on the
Program), you indicate your acceptance of this License to do so, and
all its terms and conditions for copying, distributing or modifying
the Program or works based on it.

:p.
6. Each time you redistribute the Program (or any work based on the
Program), the recipient automatically receives a license from the
original licensor to copy, distribute or modify the Program subject to
these terms and conditions.  You may not impose any further
restrictions on the recipients' exercise of the rights granted herein.
You are not responsible for enforcing compliance by third parties to
this License.

:p.
7. If, as a consequence of a court judgment or allegation of patent
infringement or for any other reason (not limited to patent issues),
conditions are imposed on you (whether by court order, agreement or
otherwise) that contradict the conditions of this License, they do not
excuse you from the conditions of this License.  If you cannot
distribute so as to satisfy simultaneously your obligations under this
License and any other pertinent obligations, then as a consequence you
may not distribute the Program at all.  For example, if a patent
license would not permit royalty-free redistribution of the Program by
all those who receive copies directly or indirectly through you, then
the only way you could satisfy both it and this License would be to
refrain entirely from distribution of the Program.

:p.
If any portion of this section is held invalid or unenforceable under
any particular circumstance, the balance of the section is intended to
apply and the section as a whole is intended to apply in other
circumstances.

:p.
It is not the purpose of this section to induce you to infringe any
patents or other property right claims or to contest validity of any
such claims; this section has the sole purpose of protecting the
integrity of the free software distribution system, which is
implemented by public license practices.  Many people have made
generous contributions to the wide range of software distributed
through that system in reliance on consistent application of that
system; it is up to the author/donor to decide if he or she is willing
to distribute software through any other system and a licensee cannot
impose that choice.

:p.
This section is intended to make thoroughly clear what is believed to
be a consequence of the rest of this License.

:p.
8. If the distribution and/or use of the Program is restricted in
certain countries either by patents or by copyrighted interfaces, the
original copyright holder who places the Program under this License
may add an explicit geographical distribution limitation excluding
those countries, so that distribution is permitted only in or among
countries not thus excluded.  In such case, this License incorporates
the limitation as if written in the body of this License.

:p.
9. The Free Software Foundation may publish revised and/or new versions
of the General Public License from time to time.  Such new versions will
be similar in spirit to the present version, but may differ in detail to
address new problems or concerns.

:p.
Each version is given a distinguishing version number.  If the Program
specifies a version number of this License which applies to it and "any
later version", you have the option of following the terms and conditions
either of that version or of any later version published by the Free
Software Foundation.  If the Program does not specify a version number of
this License, you may choose any version ever published by the Free Software
Foundation.

:p.
10. If you wish to incorporate parts of the Program into other free
programs whose distribution conditions are different, write to the author
to ask for permission.  For software which is copyrighted by the Free
Software Foundation, write to the Free Software Foundation; we sometimes
make exceptions for this.  Our decision will be guided by the two goals
of preserving the free status of all derivatives of our free software and
of promoting the sharing and reuse of software generally.

:p.
.ce NO WARRANTY

:p.
:hp8.
11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN
OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS
TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
REPAIR OR CORRECTION.

:p.
12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.
:ehp8.

:p.
.ce END OF TERMS AND CONDITIONS
:p.
.ce	    How to Apply These Terms to Your New Programs

:p.
If you develop a new program, and you want it to be of the greatest
possible use to the public, the best way to achieve this is to make it
free software which everyone can redistribute and change under these terms.

:p.
To do so, attach the following notices to the program.  It is safest
to attach them to the start of each source file to most effectively
convey the exclusion of warranty; and each file should have at least
the "copyright" line and a pointer to where the full notice is found.

:xmp.
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 19yy  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
:exmp.
:p.
Also add information on how to contact you by electronic and paper mail.

:p.
If the program is interactive, make it output a short notice like this
when it starts in an interactive mode:

:xmp.
    Gnomovision version 69, Copyright (C) 19yy name of author
    Gnomovision comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
    This is free software, and you are welcome to redistribute it
    under certain conditions; type `show c' for details.
:exmp.
:p.
The hypothetical commands `show w' and `show c' should show the appropriate
parts of the General Public License.  Of course, the commands you use may
be called something other than `show w' and `show c'; they could even be
mouse-clicks or menu items--whatever suits your program.

:p.
You should also get your employer (if you work as a programmer) or your
school, if any, to sign a "copyright disclaimer" for the program, if
necessary.  Here is a sample; alter the names:

:xmp.
  Yoyodyne, Inc., hereby disclaims all copyright interest in the program
  `Gnomovision' (which makes passes at compilers) written by James Hacker.

  <signature of Ty Coon>, 1 April 1989
  Ty Coon, President of Vice
:exmp.
:p.
This General Public License does not permit incorporating your program into
proprietary programs.  If your program is a subroutine library, you may
consider it more useful to permit linking proprietary applications with the
library.  If this is what you want to do, use the GNU Library General
Public License instead of this License.

.**********************************************************************
:h1 id=300.Installation instructions
.**********************************************************************

:p.  
&HFS. can be installed automatically using the "Device Driver
Install" object in the "System Setup" folder, or by manually editing
CONFIG.SYS.  

:p.
:sl compact.
:li.:link reftype=hd refid=310.Automated installation:elink.
:li.:link reftype=hd refid=320.Manual installation:elink.
:esl.


:h2 id=310.Automated installation

:p.
The following steps describe how to install &HFS. on OS/2 Warp.
:ol.
:li.
Open the "Device Driver Install" object. It is located in the "System
Setup" folder.
:li.
In the "Source directory" box, press the "Change" button and enter the
directory where you unpacked the &HFS. distribution archive.
:li.
In the "Destination directory" box, enter your OS/2 boot drive.
:li.
Press the "Install" button.
:li.
A list box will be displayed with the line "HFS/2 - A Hierarchical
File System Driver for OS/2". Select this line and press "OK".
:li.
Optionally add the directory d&colon.\OS2\FS\HFS (where d&colon. is your boot
drive) to the PATH statement in the CONFIG.SYS file.
:li.
Reboot the system.
:eol.

:p.  
If you intend to use a CD-ROM drive with &HFS., you will need to
make a small modification to CONFIG.SYS. See the :link reftype=hd
refid=530.usage notes:elink. for more information.

:h2 id=320.Manual installation

:p.
The following steps describe how to install &HFS. manually.
:ol.
:li.
Unpack the distribution archive hfs:hp1.nnn:ehp1..zip into a directory
anywhere on your system.
:li.
Copy the file HFSFLT.FLT to the \OS2\BOOT directory of your boot drive
for OS/2 Warp, or the \OS2 directory for earlier versions of OS/2. 
:li.
Copy the file UHFS.DLL to the \OS2\DLL directory.
:li.
Add the following line to the CONFIG.SYS file on your boot drive:
:xmp.IFS=d&colon.\path\HFS.IFS

:exmp.
where d&colon.\path\ is the path to the directory where you unpacked
the files. The line must be located after the IFS= statement for the
file system where &HFS. resides, if any. If you intend to use
&HFS. with CD-ROMs, you must also place the line before the line that
loads CDFS.IFS.
:li.
Add the line
:xmp.BASEDEV=HFSFLT.FLT

:exmp.
to the CONFIG.SYS file.
:li.
You may add the line
:xmp.RUN=d&colon.\path\HFS.EXE

:exmp.
to the CONFIG.SYS file to have the :link reftype=fn refid=control.control 
program:elink. HFS.EXE started in the background at boot time. This program
must be activated, either in this way or simply by running it from an OS/2
prompt, before using &HFS.. The control program can also be started
from STARTUP.CMD.
:li.
Reboot the system.
:eol.

.**********************************************************************
:h1 id=400.Using &HFS.
.**********************************************************************

:p.
Using &HFS. is easy. You can access a Macintosh diskette just like
an ordinary diskette. These are the steps involved in using a
Macintosh diskette with &HFS.&colon.
:ol compact.
:li.Make sure the control program is running.
:li.Insert the diskette into a diskette drive. 
:li.Access the files on the diskette in any way you like.
:li.Remove the diskette from the drive.
:eol.

:p.
&HFS. is made up of several parts:
:ul compact.
:li.The :link reftype=hd refid=410.file system driver:elink. HFS.IFS.
:li.The :link reftype=hd refid=420.control program:elink..
:li.The :link reftype=hd refid=440.filter driver:elink., HFSFLT.FLT.
:li.The :link reftype=hd refid=445.FORMAT:elink. utility which
resides in  UHFS.DLL.
:li.The utilities :link reftype=hd refid=450.SYNC.EXE:elink.
and :link reftype=hd refid=460.PREPARE.EXE:elink..
:eul.

.*********************************
:h2 id=410.The file system driver
.*********************************

:p.
The file system driver resides in the file HFS.IFS, and is loaded at
boot time through the CONFIG.SYS file. The file system driver does not
manage the HFS volumes itself, but merely calls the control program to
service all requests from the operating system.

:p.
The syntax for HFS.IFS is
:xmp.IFS=HFS.IFS [/Q]

:exmp.
where brackets indicate an optional parameter.

:dl tsize=14.
:dthd.:hp2.Option:ehp2.
:ddhd.:hp2.Description:ehp2.
:dt./Q
:dd.Suppress the banner at startup.
:edl.

.*********************************
:h2 id=420.The control program
.*********************************

:p.
The file HFS.EXE contains the :hp1.control program:ehp1.. It is the control
program that carries out most tasks on behalf of the file system
driver. HFS.EXE can be started either from an OS/2 prompt, with
a RUN= statement in CONFIG.SYS, or from STARTUP.CMD. The control
program must be running for &HFS. to be active.

:p.
HFS.EXE has the following command line syntax&colon.
:xmp.HFS.EXE [/FN&colon.mode] [/NOSYNC]

:exmp.
where brackets indicate an optional parameter.

:dl tsize=14.
:dthd.:hp2.Option:ehp2.
:ddhd.:hp2.Description:ehp2.
:dt./FN&colon.:hp1.mode:ehp1.
:dd.
Set the file name translation mode. 

:p.
This option determines how &HFS. translates Macintosh file names that
contain 8-bit characters or characters that are illegal in OS/2 file names.
:hp1.mode:ehp1. can be any of CP850, 7BIT or 8BIT. The default is
CP850. See the section on :link reftype=hd refid=510.File name
translation:elink. for details.

:dt./NOSYNC
:dd.
Do not automatically flush the volume.

:p.
Normally, &HFS. will automatically flush all buffered data after every
operation on the volume. Since this may degrade performance, this
behaviour can be turned off with the /NOSYNC option. If you use this
options, then you must run the :link reftype=hd
refid=450.SYNC:elink. utility before removing the volume from the
drive. Using this option is not recommended.
:edl.

:p.
When the control program is started, it connects to the IFS and starts
processing file system requests. Macintosh volumes can be accessed
only when the control program is running. The control program will
refuse to load if another instance of the program is already running.

:warning.  
If the control program is interrupted while a Macintosh
diskette is mounted, data may be lost and the diskette left in an
inconsistent state.
:ewarning. 
It is best to leave the control program running once it has been
started.  The control program may cause a fault and crash as a result
of a bug. If this happens, you should remove the Macintosh diskettes
immediately. It may also be necessary to run the FSKILL utility before
continuing if the control program traps. FSKILL will tell the IFS to
disconnect from the crashed control program.

.****************************
:h2 id=440.The filter driver
.****************************

:p.
Because of the way OS/2 handles diskettes, a Macintosh diskette cannot
be used directly in an OS/2 system. The diskette needs an OS/2-compatible
boot sector in order to be recognized. The :hp1.filter driver:ehp1.,
HFSFLT.FLT, is used to take care of this problem.

:p.
HFSFLT.FLT is loaded as a base device driver when the system start up. It
operates by filtering input/output requests to any diskette drives in
the system. If the operating system tries to read the boot sector of a
diskette, the filter will first check if the diskette is formatted
with HFS. If this is the case, the filter will supply a fake boot
sector to the operating system. The filter does not alter any
data on the diskette itself.

:p.  
The filter driver is new in this release of &HFS..
If the filter doesn't work on your system for any reason, an
alternative is to use the :link reftype=hd
refid=460.PREPARE:elink. program.

.*******************************
:h2 id=445.Formatting a diskette
.*******************************

:p.
&HFS. can format 1.44MB Macintosh diskettes with the Hierarchical File
System. Formatting is done through the OS/2 FORMAT command. The syntax
is&colon.
:xmp.FORMAT d&colon. /FS&colon.HFS [/V&colon.:hp1.label:ehp1.]

:exmp.
where d&colon. is the drive to format, and brackets indicate
an optional parameter.

:dl tsize=14.
:dthd.:hp2.Option:ehp2.
:ddhd.:hp2.Description:ehp2.
:dt./V&colon.:hp1.label:ehp1.
:dd.
Set the volume label.
:edl.

:note.  
The formatting program does not support partitioned media. It can only
format single-volume media, which means 1.44MB diskettes. Future
version of &HFS. may have more capabilities.

:p.  
&HFS. does not perform a low-level format. Therefore, the
diskette must already be formatted, for example with the FAT file
system, before it is formatted with &HFS..

:hp8.
:warning.
Do not attempt to format any other media than 1.44MB diskettes with
&HFS.. If you do, the data on the media will be destroyed and the
media will not be usable.
:ewarning.
:ehp8.

.*********************************
:h2 id=450.Synchronizing a volume
.*********************************

:p.
When an HFS volume is accessed, the file system driver keeps 
some HFS data structures
in memory for performance reasons. Before the diskette is removed from
the drive, these structures must be written to the volume. This can be
done by running the program SYNC.EXE.

:note.
You do not normally need to run SYNC, since &HFS. will write any
buffered data to the volume immediately. You need to run SYNC only if
you use the /NOSYNC option to the control program.

:p.
Just type
:xmp.SYNC d&colon.

:exmp.  
at a command prompt to sync the diskette in drive d&colon..
If you omit the drive letter, SYNC will sync all HFS volumes that are
currently mounted.  If the sync succeeds, the program will print a
message saying that the diskette may be removed.

:warning.
If you run HFS.EXE with the /NOSYNC options, and remove an HFS
diskette from the drive without running SYNC first, 
the diskette may be left in an inconsistent state. 
:ewarning.

.************************************************
:h2 id=460.Preparing a diskette for use with &HFS.
.************************************************

:p.
In previous versions of &HFS., the PREPARE utility was used
instead of the :link reftype=hd refid=440.filter driver:elink..
It is no longer necessary to use PREPARE, and it is included
here only in case there are problems with the filter driver.

:p.
PREPARE.EXE writes an OS/2-style boot sector to the first sector of
the diskette. The syntax is
:xmp.PREPARE /P d&colon.

:exmp.
where d&colon. is the drive with the diskette to be prepared. If the
drive letter is omitted, PREPARE will assume drive A&colon..

:p.
PREPARE.EXE will save the original boot sector in a data file in the current
directory. The name  will be the hexadecimal serial number of the
diskette. If a file with that name already exists, PREPARE will complain
and not prepare the diskette.

:p.
In order to restore the boot sector from this data file, use the command
:xmp.PREPARE /R d&colon.

:exmp.
This will copy the saved original boot sector from the data file back
to the diskette. The data file must be found in the current directory,
or PREPARE will complain.

:p.
Macintosh diskettes usually do not need a boot sector unless they are
used to boot from. An HFS diskette will work in a Macintosh computer
even if it has an OS/2-style boot sector. If you do not intend to restore
a diskette, you can delete the data file generated by PREPARE.

:note.
If the Macintosh has PC Exchange installed, then you will need to
restore the boot sector before using the diskette on the Macintosh.

.**********************************************************************
:h1 id=500.Usage notes
.**********************************************************************
:sl compact.
:li.:link reftype=hd refid=510.File name translation:elink.
:li.:link reftype=hd refid=520.Icon conversion:elink.
:li.:link reftype=hd refid=530.Using a CD-ROM:elink.
:esl.

.********************************
:h2 id=510.File name translation
.********************************

:p.
The rules for naming files differ between OS/2 and Macintosh system.
The most notable difference is that Macintosh file names cannot exceed
31 characters in length. Certain characters, such as '/' and '\', are
not allowed in OS/2 file names, but are perfectly valid in HFS file names.
Moreover, the Macintosh uses a character set different from those found
on OS/2 systems.

:p.
To accomodate these differences, some sort of file name translation is
necessary. &HFS. supports several translation modes. The translation
mode can be set with a command line option to the :link reftype=hd
refid=420.control program:elink..

:p.
The following translation modes are supported.
:parml tsize=12 break=none.
.***
:pt.:hp4.CP850:ehp4.
:pd.This mode of translation tries to map characters in the Macintosh
character set to their equivalents in codepage 850. 
It is the default translation mode for &HFS..
:parml tsize=3.
:pt.Macintosh to OS/2&colon.
:pd.:ul compact.
:li.All valid characters in the range 32-127 except '%' and a trailing
dot remain unchanged.
:li.Characters in the range 128-255 that have an equivalent in codepage 850
map to that character.
:li.All other characters are replaced by the three characters 
'%xx', where xx is the character's hexadecimal representation.
:eul.
:pt.OS/2 to Macintosh&colon.
:pd.:ul compact.
:li.Characters 32-127 except '%' remain unchanged.
:li.Characters in the range 128-255 that have a Macintosh equivalent map
to it.
:li.The sequence '%xx' where xx is the hexadecimal representation of '%' 
becomes the '%' character. If xx is a hexadecimal number in the range
128-255 that does not have a Macintosh equivalent, 
then the sequence is replaced with that character. If xx is the
hexadecimal code for a dot, and there are no more characters in the
filename, the sequence is replaced with a dot.
All other combinations of characters
are illegal, and will be rejected by &HFS..
:eul.
:eparml.
:p.
Note that some OS/2 file names will not be permitted by &HFS. with
this mode of translation, even though they may be short enough. This
is the price for eliminating name clashes.
.***
:pt.:hp4.7BIT:ehp4.
:pd.This translation mode quotes all characters in the range 128-255.
:parml tsize=3.
:pt.Macintosh to OS/2&colon.
:pd.:ul compact.
:li.Valid characters in the range 32-127 except '%' and a trailing dot
remain unchanged.
:li.All other characters are replaced by the three characters 
'%xx', where xx is the character's hexadecimal representation.
:eul.
:pt.OS/2 to Macintosh&colon.
:pd.:ul compact.
:li.Characters 32-127 except '%' remain unchanged.
:li.The sequence '%xx' where xx is the hexadecimal representation of
'%', a character in the range 32-127 not allowed in OS/2 file names, a
character in the range 128-255, or a trailing dot is mapped to the
corresponding character. In other cases, including when xx is not a hexadecimal
number, the combination is illegal, and will be rejected by &HFS..
:eul.
:eparml.
.***
:pt.:hp4.8BIT:ehp4.
:pd.This translation mode leaves most characters unchanged, even in the range
128-255.
:parml tsize=3.
:pt.Macintosh to OS/2&colon.
:pd.:ul compact.
:li.Valid characters in the range 32-255 except '%' and a trailing dot
remain unchanged.
:li.All other characters are replaced by the three characters 
'%xx', where xx is the character's hexadecimal representation.
:eul.
:pt.OS/2 to Macintosh&colon.
:pd.:ul compact.
:li.Characters 32-255 except '%' remain unchanged.
:li.The sequence '%xx' where xx is the hexadecimal representation of
'%', a character not allowed in OS/2 file names, or a trailing dot
is mapped to that character. 
In other cases, the combination is illegal, and will be rejected by &HFS..
:eul.
:eparml.
:eparml.

.**************************
:h2 id=520.Icon conversion
.**************************

:p.  
Some Macintosh files have icons attached to them. &HFS. will
convert any such :hp1.custom icons:ehp1. to the OS/2 format, 
so that they are displayed in the Workplace Shell.
The conversion is completely
transparent, and doesn't alter any data on the HFS volume.
The converted icons are presented to the system as extended attributes.

:p.
Note that not all files have custom icons attached. Most files use
shared icons stored in a special resource file. This version of
&HFS. does not convert those icons.


.************************
:h2 id=530.Using a CD-ROM
.************************

:p.
Many CD-ROMs have a "hybrid" file system, with one HFS partition
visible on the Macintosh, and one ISO partition visible on other
systems. In order to access the HFS partition on such a CD-ROM,
&HFS. must get to see the CD-ROM before the built-in CD-ROM file
system driver does. This can be achieved by placing the line
:xmp.IFS=d&colon.\path\HFS.IFS

:exmp.
:hp1.before:ehp1. the line
:xmp.IFS=d&colon.\path\CDFS.IFS

:exmp.
in the CONFIG.SYS file.

:note.
Be sure to place the line for HFS.IFS :hp1.after:ehp1. the driver for
the file system that HFS.IFS resides in, or you will not be able to
boot the system. For example, if HFS.IFS resides in an HPFS partition,
you would load HPFS.IFS first, followed by HFS.IFS, followed by
CDFS.IFS.

:p.  
If you want to access the HFS file system on a hybrid CD-ROM but
CDFS is already mounted, remove the CD-ROM from the
drive, and then try to access it in some way so that an error message
is generated. This will unmount CDFS, and enable &HFS. to mount the CD-ROM
when it is inserted. This is typically necessary when the CD-ROM
is in the drive during system startup.

.**********************************************************************
:h1 id=600.Building the sources
.**********************************************************************

:p.
The following sections describe how the sources of &HFS. are
organized, and what tools are needed to build the program. You do not
need to read this section unless you intend to compile &HFS. yourself.

:sl compact.
:li.:link reftype=hd refid=610.The tools needed:elink.
:li.:link reftype=hd refid=615.The sources:elink.
:li.:link reftype=hd refid=620.The Ring 0 piece:elink.
:li.:link reftype=hd refid=630.The Ring 3 piece:elink.
:esl.

.**************************
:h2 id=610.The tools needed
.**************************

:p.
To build &HFS., you will need two C compilers&colon. a 16-bit compiler
for the IFS, filter driver and file system utilites DLL, and a 32-bit
compiler for the control program and the 
utilities. Small modifications to the sources and makefiles may
be necessary if you use different compilers than the ones listed below.
Some modifications to the makefiles will be necessary either way in order
to adjust them to your system.

:p.
I used the following tools to develop &HFS..
:ul compact.
:li.
emx 0.9c for the 32-bit parts.
:li.
Borland C++ 3.1 and Turbo Assembler 3.2 for the 16-bit parts.
:li.
GNU Make version 3.75.
:li.
The Device Driver Kit (DDK) from IBM, which is available at
:hp1.http&colon.//service.boulder.ibm.com/ddk:ehp1..
:li.
The IFS Toolkit, which is available as
:hp1.ftp&colon.//ftp-os2.nmsu.edu/os2/dev32/ifsinf.zip:ehp1..
:eul.

.**************************
:h2 id=615.The sources
.**************************

:p.
The file hfssrc.zip contains the source tree. Some of the source files
have long file names, so you will have unpack it to an HPFS drive.
The directories in the tree are as follows:

:p.
:parml compact break=all.
:pt..
:pd.The root directory of the development tree contains header
files shared by the ring 0 and ring 3 pieces, as well as the 
top-level Makefile which is only used to create the distribution
archive.
:pt..\Ring0
:pd.This directory contains the sources for the ring 0 piece (the IFS).
:pt..\Ring3
:pd.This directory contains the sources for the control program.
:pt..\hfsflt
:pd.The sources for the filter.
:pt..\uhfs
:pd.The sources for the file system utilities DLL.
:pt..\util
:pd.The sources for the utilities.
:pt..\doc
:pd.The documentation source.
:eparml.

.**************************
:h2 id=620.The Ring 0 piece
.**************************

:p.
The .\Ring0 directory in the development tree contains C and asm sources,
and a makefile for Borland's MAKE utility. You will
have to edit the makefile so that the paths are correct for your
system.

:p.
The makefile contains compiler options for BCC. I have
sometimes experienced unexpected results after changing some
of these options, in particular those related to optimizations.

:p.
If the DEBUG symbol is defined, then the IFS will print some
debugging information through a logging facility. The messages
are printed out by the logging thread of the control program
if it is also compiled with DEBUG defined. 

.**************************
:h2 id=630.The Ring 3 piece
.**************************

:p.
The .\Ring3 directory contains the sources for the control
program. The libhfs subdirectory contains a slightly modified version
of Robert Leslie's libhfs.

:p.
Dependencies can be generated by doing a
:xmp.make dep

:exmp.
This generates a dependency file called .depend in each subdirectory. 
However, the makefiles require the .depend files to be present
from the beginning, so you will have to create an empty .depend
file in each directory manually before you can run make. 
There are better ways to write
makefiles to avoid this, I just haven't got around to it yet.

:p.
The TOPDIR variable in .\Ring3\Makefile will need to be modified
to suit your system. When that is done, just type
:xmp.make

:exmp.
to compile everything.

:p.
If the symbol DEBUG is defined, HFS.EXE will print some information
on its actions. It will also create a logging thread which prints
messages from the IFS.

.**********************************************************************
:h1 id=800.History of changes
.**********************************************************************
:sl compact.
:li.:link reftype=hd refid=810.Changes in version 0.02:elink.
:li.:link reftype=hd refid=820.Changes in version 0.10:elink.
:esl.

.*********************************
:h2 id=810.Changes in version 0.02
.*********************************

:p.
These are the most important changes and new features in &HFS. version
0.02.

:ul compact.
:li.Works with CD-ROMs.
:li.Added file name translation to convert file names which are
invalid in OS/2.
:li.Hidden files are now supported.
:li.Implemented move operations.
:li.Drag and drop works in the Workplace Shell.
:li.Macintosh icons are transparently converted to OS/2 format (custom
icons only).
:li.A filter driver, HFSFLT.FLT, replaces PREPARE.
:li.SYNC and PREPARE now use drive A&colon. by default.
:li.Fixed a bug in MacFS which would cause the control program to
crash after a disk change.
:li.Added an interface for other programs to communicate with
&HFS. through DosFSCtl. Currently, this allows programs to read the
resource fork of a file.
:eul.

.*********************************
:h2 id=820.Changes in version 0.10
.*********************************

:p.
These are the most important changes and new features in &HFS. version
0.10.

:ul compact.
:li.Switched to Robert Leslie's HFS implementation.
:li.&HFS. can now format 1.44MB floppies.
:li.It is no longer necessary to run the SYNC program before removing
a diskette.
:li.Trailing dots in filenames are properly handled.
:li.Fixed a bug with the filter driver that caused problems with some
removable drives.
:li.Added /Q command line option to HFS.IFS to suppress the banner.
:eul.

.**********************************************************************
.* Footnotes
.**********************************************************************

:fn id=control.
The :hp1.controlpProgram:ehp1., HFS.EXE, is the part of &HFS. that runs
in ring 3 (user privilege level). In order to access a Macintosh
diskette, the control program must be running.
:efn.

:fn id=diskette.
:p.
The only type of diskettes supported are 1.44MB diskettes 
formatted with the :hp1.Hierarchical File System:ehp1. (HFS).  
400K and 800K Macintosh diskettes cannot be used with the PC hardware.
:efn.

:euserdoc.
