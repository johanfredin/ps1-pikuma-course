all:
	make build
	make exe

clean:
	rm ex1.bin
	rm ex1.ps-exe

build:
	armips ex1.asm

exe:
	python3 ../../bin2exe.py ex1.bin ex1.ps-exe
