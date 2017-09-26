SRC:=main.c
RC:=WebNotifyResources.rc
OBJ:=$(patsubst %.c,%.o,$(SRC)) $(patsubst %.rc,%.o,$(RC))
CCFLAGS:=-Wall -Wpedantic -g
LDFLAGS:=-mwindows
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
	
%.o: %.rc
	windres -i $< -o $@

.PHONY: clean
clean:
	cmd /c "del $(EXE)"
	cmd /c "del *.o"
	cmd /c "del *.d"



