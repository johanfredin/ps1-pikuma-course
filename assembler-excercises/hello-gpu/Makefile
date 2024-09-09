all:
	make build
	make exe

clean:
	rm *.bin
	rm *.ps-exe

build:
	armips hello-gpu.asm

exe:
	python3 ../../bin2exe.py hello-gpu.bin hello-gpu.ps-exe
