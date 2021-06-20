#include "system.h"
#define PIC1_DATA 0x21
#define PIC2_DATA 0xA1
bool IsIntInit = false;
bool test = true;

extern "C" void SyscallEntry(InterruptStack* Registers, uint8_t CoreID){
    uint64_t syscall = (uint64_t)Registers->rax;
    uint64_t arg0 = (uint64_t)Registers->rdi;
    uint64_t arg1 = (uint64_t)Registers->rsi;
    uint64_t arg2 = (uint64_t)Registers->rdx;
    uint64_t arg3 = (uint64_t)Registers->r10;
    uint64_t arg4 = (uint64_t)Registers->r8;
    uint64_t arg5 = (uint64_t)Registers->r9;

    switch(syscall){
        case 0x01:
            break;
        case 0xff:
            if(arg0 == arg1 && arg1 == arg2 && arg2 == arg3 && arg3 == arg4 && arg4 == syscall){
               globalTaskManager.IsEnabledPerCore[CoreID] = true; 
               for(int i = 0; i < APIC::ProcessorCount; i++){
                   if(!globalTaskManager.IsEnabledPerCore[i]) return;
                   globalTaskManager.IsEnabled = true; 
               }
            }
    }
    printf("%s", arg5);

    globalGraphics->Update(); 

    Registers->r8 = (void*)GDTInfoSelectors.UCode;
    Registers->r9 = (void*)GDTInfoSelectors.UData;
}

extern "C" uint64_t SystemExit(uint64_t ErrorCode){

}

