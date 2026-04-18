.text
main:
  addi a0, zero, 5
  addi a1, zero, 3
  add a2, a0, a1
  jal ra, finish
finish:
  jal x0, finish  # infinite loop
