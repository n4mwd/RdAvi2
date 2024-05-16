#
# Borland C++ IDE generated makefile
# Generated 5/14/2024 at 12:54:42 PM
#
.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCC32   = Bcc32 +BccW32.cfg
BCC32I  = Bcc32i +BccW32.cfg
TLINK32 = TLink32
ILINK32 = Ilink32
TLIB    = TLib
BRC32   = Brc32
TASM32  = Tasm32
#
# IDE macros
#


#
# Options
#
IDE_LinkFLAGS32 =  -LC:\BC5\LIB
LinkerLocalOptsAtC32_rdavi2dexe =  -Tpe -ap -c
ResLocalOptsAtC32_rdavi2dexe =
BLocalOptsAtC32_rdavi2dexe =
CompInheritOptsAt_rdavi2dexe = -IC:\BC5\INCLUDE
LinkerInheritOptsAt_rdavi2dexe = -x
LinkerOptsAt_rdavi2dexe = $(LinkerLocalOptsAtC32_rdavi2dexe)
ResOptsAt_rdavi2dexe = $(ResLocalOptsAtC32_rdavi2dexe)
BOptsAt_rdavi2dexe = $(BLocalOptsAtC32_rdavi2dexe)

#
# Dependency List
#
Dep_rdavi2 = \
   rdavi2.exe

rdavi2 : BccW32.cfg $(Dep_rdavi2)
  echo MakeNode

Dep_rdavi2dexe = \
   rdavi2.obj\
   codecs.obj\
   file64.obj\
   fileutil.obj

rdavi2.exe : $(Dep_rdavi2dexe)
  $(ILINK32) @&&|
 /v $(IDE_LinkFLAGS32) $(LinkerOptsAt_rdavi2dexe) $(LinkerInheritOptsAt_rdavi2dexe) +
C:\BC5\LIB\c0x32.obj+
rdavi2.obj+
codecs.obj+
file64.obj+
fileutil.obj
$<,$*
C:\BC5\LIB\import32.lib+
C:\BC5\LIB\cw32.lib



|
Dep_rdavi2dobj = \
   rdavi2.h\
   rdavi2.c

rdavi2.obj : $(Dep_rdavi2dobj)
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_rdavi2dexe) $(CompInheritOptsAt_rdavi2dexe) -o$@ rdavi2.c
|

codecs.obj :  codecs.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_rdavi2dexe) $(CompInheritOptsAt_rdavi2dexe) -o$@ codecs.c
|

file64.obj :  file64.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_rdavi2dexe) $(CompInheritOptsAt_rdavi2dexe) -o$@ file64.c
|

fileutil.obj :  fileutil.c
  $(BCC32) -P- -c @&&|
 $(CompOptsAt_rdavi2dexe) $(CompInheritOptsAt_rdavi2dexe) -o$@ fileutil.c
|

# Compiler configuration file
BccW32.cfg :
   Copy &&|
-w
-R
-v
-WM-
-vi
-H
-H=readavi.csm
-WC
| $@


