void main(int test){
    asm(".att_syntax prefix");
    asm(".intel_syntax noprefix");
    asm("movq rax, 0"); //syscall number
    asm("movq rdi, 0"); //argument 0
    asm("movq rsi, 0"); //argument 1
    asm("movq rdx, 1"); //argument 2
    asm("movq r10, 0"); //argument 3
    asm("movq r8, 255");  //argument 4
    asm("movq r9, 0");  //argument 5

    asm(".att_syntax prefix");  
    
    asm("int $0x80"); 
    while(true);
}