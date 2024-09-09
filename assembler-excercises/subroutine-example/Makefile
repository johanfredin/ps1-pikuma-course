all:
	make build
	make exe

clean:
	rm subroutine.bin
	rm subroutine.ps-exe


build:
	armips subroutine.asm

exe:
	python3 ../../bin2exe.py subroutine.bin subroutine.ps-exe
