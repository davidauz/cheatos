CC     = gcc
CFLAGS = -g -mwindows
headers = definitions.h logic.h
cheatos_src = cheatos.c logic.c definitions.c getbaseaddress.o
cheatos_obj=$(cheatos_src:%.c=%.o)

all: cheatos.exe cheatos.dll
	@echo "ALL DONE"

cheatos.exe : $(cheatos_obj) $(headers) app.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(cheatos_obj) app.o -lwinmm

cheatos.dll : dll_injection.o logic.o memscan.o definitions.o getbaseaddress.o $(headers)
	$(CC) -shared -o cheatos.dll dll_injection.o logic.o memscan.o definitions.o getbaseaddress.o  -lwinmm

# .c.o:
%.o: %.c $(headers)
	$(CC) $(CFLAGS) -g -c $<

app.o: app.rc
	windres app.rc app.o

cheatos.o: $(headers)
dll_injection.o: $(headers)
logic.o: $(headers)
app.o: app.rc

clean:
	rm -f *.exe
	rm -f *.o
	rm -f *.dll

