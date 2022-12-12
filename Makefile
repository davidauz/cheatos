CC     = gcc
CFLAGS = -g -mwindows
headers = definitions.h logic.h
cheatos_src = cheatos.c logic.c
cheatos_obj=$(cheatos_src:%.c=%.o)

all: cheatos.exe cheatos.dll
	@echo "ALL DONE"

cheatos.exe : $(cheatos_obj) $(headers)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(cheatos_obj)

cheatos.dll : dll_injection.o logic.o $(headers)
	$(CC) -shared -o cheatos.dll dll_injection.o logic.o

# .c.o:
%.o: %.c $(headers)
	$(CC) $(CFLAGS) -g -c $<

cheatos.o: $(headers)
dll_injection.o: $(headers)
logic.o: $(headers)

clean:
	del *.exe
	del *.dll
	del *.o

