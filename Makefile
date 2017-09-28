SRC:=main.c server.c
RC:=WebNotifyResources.rc
RCO:=$(patsubst %.rc,%.o,$(RC))
RES:=$(wildcard resources/*.*)
OBJ:=$(patsubst %.c,%.o,$(SRC)) $(RCO)
CCFLAGS:=-Wall -Wpedantic -g -DWINVER=0x0501 -D_WIN32_WINNT=0x0501 -D_WIN32_IE=0x0500
LDFLAGS:=-lws2_32 -mwindows
EXE:=WebNotify.exe

.PHONY: all
all: $(EXE)

$(EXE): $(OBJ)
	gcc -o $@ $(OBJ) $(LDFLAGS)

-include $(SRC:.c=.d)

%.d: %.rc
	gcc -MM -MG $< > $@

%.d: %.c
	gcc -MM -MG $< > $@

%.o: %.c
	gcc $(CCFLAGS) -c $< -o $@
	
$(RCO): $(RC) resource.h $(RES)
	windres -i $< -o $@

.PHONY: clean
clean:
	cmd /c "DEL /Q $(EXE)"
	cmd /c "DEL /Q *.o"
	cmd /c "DEL /Q *.d"

.PHONY: tags
tags: $(SRC)
	ctags -V --recurse
