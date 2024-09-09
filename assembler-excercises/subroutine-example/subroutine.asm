.psx
.create "factorial.bin", 0x80010000

.org 0x80010000

Main:
  li $a0, 6
  jal Factorial  ; jump and link to subroutine, caller addr stored in $ra
  nop

LoopForever:
  j LoopForever
  nop

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Subroutine to compute the factorial of a number
;; Argument: num ($a0)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Factorial:
  li $t3, 1                   ; temp = 1
  li $t4, 1                   ; sum = 0
  li $t1, 1                   ; i = 1
OuterLoop:
  bgt $t1, $a0, EndOuterLoop  ; break if (i > num)
  nop
  li $t4, 0                   ; sum = 0 
  li $t2, 0                   ; j = 0
InnerLoop:
  bge $t2, $t1, EndInnerLoop  ; break if (j >= i)
  nop
  add $t4, $t4, $t3           ; sum += temp
  addi $t2, $t2, 1            ; j++
  j InnerLoop                 ; Jump back up to the inner loop
  nop
EndInnerLoop:
  move $t3, $t4               ; temp = sum

  addi $t1, $t1, 1            ; i++
  j OuterLoop                 ; Jump back up to the outer loop
  nop
EndOuterLoop:
  move $v0, $t4               ; $v0 receives the final result of the factorial (sum)
  jr $ra                      ; return from the subroutine

.close
