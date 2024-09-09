; Your goal here is to start $t0 with 1 and loop 10 times, incrementing $t0. At the end, you
; should have the accumulated sum of all values (1+2+3+4+5+6+7+8+9+10) inside $t1


; TODO:
; 1. Start $t0 with the value 1 and $t1 with the value 0
; 2. Loop, incrementing $t0 until it reaches the value 10
; 3. Keep adding and accumulating all values of $t0 inside $t1

; Tell the assembler what system to build for
.psx 
.create "ex2.bin", 0x80010000

; Entry point of Code
.org 0x80010000

Main:
  li $t0, 0x1 		; $t0 = 1
  move $t1, $zero	; $t1 = 0

Loop:
  add $t1, $t1, $t0	; $t1 += $t0
  addi $t0, $t0, 0x1	; $t0++
  ble $t0, 0xA, Loop	; while($t0 <= 10)
  nop			; nop is required after branch

End:


.close
