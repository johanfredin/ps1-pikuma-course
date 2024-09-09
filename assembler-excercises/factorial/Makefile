all:
	make build
	make exe

clean:
	rm factorial.bin
	rm factorial.ps-exe


build:
	armips factorial.asm

exe:
	python3 ../../bin2exe.py factorial.bin factorial.ps-exe
