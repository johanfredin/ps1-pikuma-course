; Tell the assembler what system to build for
.psx 
.create "fill-mem.bin", 0x80010000


; Entry point of Code
.org 0x80010000

; Constant declaration
BASE_ADDR equ 0x0000

Main:
  li $t0, 0xA000		; $t0 = 0xA000
  li $t1, 0xA0FF		; $t1 = 0xA0FF
  li $t2, 0x12345678		; $t2 = 0x12345678

Loop:
  sw   $t2, BASE_ADDR($t0)	; Store word at 0x0000 + t0
  addi $t0, $t0, 4		; t0 += 4 (sizeof(32bit word))
  ble $t0, $t1, Loop		; while (t0 < t1) keep looping

End:

.close
