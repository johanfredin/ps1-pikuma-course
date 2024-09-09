all:
	make build
	make exe

clean:
	rm ex2.bin
	rm ex2.ps-exe

build:
	armips ex2.asm

exe:
	python3 ../../bin2exe.py ex2.bin ex2.ps-exe
