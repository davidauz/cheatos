CC     = gcc
CFLAGS = -g -municode -mwindows

all: cheatos.exe cheatos.dll
	@echo "ALL DONE"

cheatos.exe : cheatos.o logic.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ cheatos.o logic.o 

cheatos.dll : dll_injection.o logic.o
	$(CC) -shared -o cheatos.dll dll_injection.o logic.o

.c.o:
	$(CC) $(CFLAGS) -g -c $<

clean:
	del *.exe
	del *.dll
	del *.o

