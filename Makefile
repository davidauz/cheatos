CC     = gcc
CFLAGS = -g -municode -mwindows

cheatos.exe : cheatos.o logic.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ cheatos.o logic.o 

.c.o:
	$(CC) $(CFLAGS) -g -c $<


clean:
	del *.exe
	del *.o

