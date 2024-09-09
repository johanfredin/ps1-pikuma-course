.psx
.create "factorial.bin", 0x80010000

.org 0x80010000

num equ 6
sum equ 0x8000a000

; $t1 = i
; $t2 = j
; $t3 = temp
; $t4 = sum 

Main:
  li $t3, 1		; temp = 1
  li $t4, 1		; sum = 1
  li $t1, 1		; i = 1

OuterWhile:                       ;   do {
  move $t4, $zero		              ;     sum = 0
  move $t2, $zero                 ;     j = 0
InnerWhile:			                  ;     do {    
  addu $t4, $t3     	            ;       sum += temp
  addiu $t2, 1    	              ;       j++
  blt $t2, $t1, InnerWhile        ;     } while (j < i)
  nop   

  move $t3, $t4		                ;     temp = sum
  addiu $t1, 1    	              ;     i++
  ble $t1, num, OuterWhile        ;   } while (i <=) num)
  nop
                               
  sw $t4, sum
End:                              ;   return num;

.close
