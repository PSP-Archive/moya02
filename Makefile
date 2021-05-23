
TITLE = "GLVOX Ver0.01(for EDUCATION)"

# makefile 
# please at the first time only type to 'make maketree' on bash console.

#add tools
RANLIB = psp-ranlib
MD = -mkdir.exe

BINARY   = moya.out
BINARY_P = moya.outp
OUTPATCH = ./outpatch.exe
ELF2PBP  = ./elf2pbp.exe
ICON0    = ICON0.PNG

CC     = psp-gcc-4.0.0
CPP    = psp-gcc-4.0.0
LINKER = psp-gcc-4.0.0
STRIP  = psp-strip

AS_OPT   = -mgp32 -mlong32 -msingle-float -mabi=eabi
CC_OPT   = -O3 -fomit-frame-pointer -fno-exceptions -mgp32 -mlong32 -mabi=eabi -c
CPP_OPT  = -O3 -fomit-frame-pointer -fno-exceptions -mgp32 -mlong32 -mabi=eabi -c
LINK_OPT = -nostartfiles -Wl,-Ttext=0x08900000
#LINK_OPT= -nostartfiles     -Ttext=0x08900000 -M
DEFINES  = -D_PSP10 -D_SCE_IO
MDCORE   = 1
#PCECORE = 1
# -D_USE_ZLIB
DEFINES += -D_USE_MD -DCOMPILE_WITH_MUSA

OBJ = obj
OBJDIRS = \
$(OBJ) 

OBJS  = \
$(OBJ)/startup.o \
$(OBJ)/glvox.o \
$(OBJ)/pg.o \
$(OBJ)/_clib.o 


LIBRARY = 

all: $(BINARY)

$(BINARY): $(OBJS)
	$(LINKER) $(OBJS) $(LIBRARY) $(LINK_OPT) -o $@
	$(STRIP) $(BINARY)
	$(OUTPATCH) $(BINARY) $(BINARY_P) "USERPROG"
	$(ELF2PBP) $(BINARY_P) $(TITLE) $(ICON0)

$(OBJ)/%.o : src/%.c
	$(CC) $(CC_OPT) $(DEFINES) $< -o $@

$(OBJ)/%.o : src/%.S
	$(CC) $(DEFINES) $(AS_OPT) -c $< -o $@

$(sort $(OBJDIRS)):
	$(MD) $@

maketree: $(sort $(OBJDIRS))

clean:
	$(RM) $(BINARY)
	$(RM) $(BINARY_P)
	$(RM) $(OBJ)/*.o

