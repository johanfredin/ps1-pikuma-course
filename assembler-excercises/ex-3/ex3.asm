; This exercise requires you to translate a small C code into MIPS assembly. The goal of
; this short code is to perform integer division between two numbers. We declare three
; variables in our C code: num (numerator), den (denominator), and res (final result of the
; integer division). You should assume the values of num, den, and res are stored into
; $t0, $t1, and $t2, respectively.

/* C code: */
;main() {
;  int num; // Assume num is loaded in $t0
;  int den; // Assume den is loaded in $t1
;  int res; // Assume res is loaded in $t2
;  num = 27; // Or any other number that we want
;  den = 3; // Or any other number that we want
;  res = 0;
;  while (num >= den) {
;    num -= den;
;    res++;
;  }
;}

.psx
.create "ex3.bin", 0x80010000
.org 0x80010000

Main:
  move $t2, $zero		; $t2 = 0
  li $t0, 27			; $t0 = 27
  li $t1, 3			; $t1 = 3
While:	
  blt $t0, $t1, EndWhile	; if ($t0 < $t1) Branch to EndWhile
  nop
  subu $t0, $t0, $t1		; t0 -= $t1
  addiu $t2, $t2, 1		; t2++
  b While			; Unconditional branch to While label
  nop
EndWhile:

End:
.close
