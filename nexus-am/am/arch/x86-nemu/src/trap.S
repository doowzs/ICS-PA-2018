#----|-------entry-------|-errorcode-|---irq id---|---handler---|
.globl vecsys;    vecsys:  pushl $0;  pushl $0x80; jmp asm_trap
.globl vectrap;  vectrap:  pushl $0;  pushl $0x81; jmp asm_trap
.globl irq0;        irq0:  pushl $0;  pushl $0x20; jmp asm_trap
.globl vecnull;  vecnull:  pushl $0;  pushl   $-1; jmp asm_trap

asm_trap:
  pushal

  # pushl $0       # for prot
	mov %cr3, %eax
	pushl %eax

  pushl %esp
  call irq_handle
	
  #addl $4, %esp
	mov %eax, %esp

  addl $4, %esp
  popal
  addl $8, %esp

  iret
