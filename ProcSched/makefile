main.out: main.o process.o edf.o cpu.o cc.o process.h edf.h
	gcc -o main.out process.o main.o edf.o cpu.o cc.o

process.o: process.c process.h
	gcc -c process.c

edf.o: edf.c edf.h
	gcc -c edf.c

cpu.o: cpu.c cpu.h
	gcc -c cpu.c

cc.o: cc.c cc.h
	gcc -c cc.c

clean:
	rm -f *.o *.out