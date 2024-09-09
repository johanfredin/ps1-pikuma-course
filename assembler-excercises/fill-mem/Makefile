all:
	make build
	make exe

clean:
	rm *.bin
	rm *.ps-exe

build:
	armips fill-mem.asm

exe:
	python3 ../../bin2exe.py fill-mem.bin fill-mem.ps-exe
