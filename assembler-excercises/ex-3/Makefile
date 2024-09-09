all:
	make build
	make exe

clean:
	rm ex3.bin
	rm ex3.ps-exe

build:
	armips ex3.asm

exe:
	python3 ../../bin2exe.py ex3.bin ex3.ps-exe
