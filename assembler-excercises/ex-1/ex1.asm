; EX - 1 TODO:
; 1. Load $t0 with the immediate decimal value 1
; 2. Load $t1 with the immediate decimal value 256
; 3. Load $t2 with the immediate decimal value 17


; Tell the assembler what system to build for
.psx 
.create "ex1.bin", 0x80010000

; Entry point of Code
.org 0x80010000

; Constant declaration
BASE_ADDR equ 0x0000

Main:
  li $t0, 0x1 
  li $t1, 0x100
  li $t2, 0x11

End:

.close
