all:
	make build
	make exe

clean:
	rm fill-mem.bin
	rm fill-mem.ps-exe

build:
	armips fill-mem.asm

exe:
	python3 bin2exe.py fill-mem.bin fill-mem.ps-exe
