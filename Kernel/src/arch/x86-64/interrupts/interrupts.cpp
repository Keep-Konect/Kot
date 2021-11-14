#include "interrupts.h"

IDTR idtr;

uint8_t IDTData[0x1000];

void InitializeInterrupts(){
    if(idtr.Limit == 0){
        idtr.Limit = 0x0FFF;
        idtr.Offset = (uint64_t)&IDTData[0];
    }

    /* Exceptions */

    /* Divide-by-zero */
    SetIDTGate((void*)Entry_DivideByZero_Handler, 0x0, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    
    /* Debug */
    SetIDTGate((void*)Entry_Debug_Handler, 0x1, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    
    /* NMI */
    SetIDTGate((void*)Entry_NMI_Handler, 0x2, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    
    /* Breakpoint */
    SetIDTGate((void*)Entry_Breakpoint_Handler, 0x3, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    
    /* Overflow */
    SetIDTGate((void*)Entry_Overflow_Handler, 0x4, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    
    /* Bound Range Exceeded */
    SetIDTGate((void*)Entry_BoundRangeExceeded_Handler, 0x5, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    
    /* Invalid Opcode */
    SetIDTGate((void*)Entry_InvalidOpcode_Handler, 0x6, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    
    /* Device Not Available */
    SetIDTGate((void*)Entry_DeviceNotAvailable_Handler, 0x7, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* Double Fault */
    SetIDTGate((void*)Entry_DoubleFault_Handler, 0x8, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* Invalid TSS */
    SetIDTGate((void*)Entry_InvalidTSS_Handler, 0xA, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* Segment Not Present */
    SetIDTGate((void*)Entry_SegmentNotPresent_Handler, 0xB, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* Stack-Segment Fault */
    SetIDTGate((void*)Entry_StackSegmentFault_Handler, 0xC, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* GP Fault */
    SetIDTGate((void*)Entry_GPFault_Handler, 0xD, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* Page Fault */
    SetIDTGate((void*)Entry_PageFault_Handler, 0xE, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* x87 Floating-Point Exception */
    SetIDTGate((void*)Entry_x87FloatingPointException_Handler, 0x10, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* Alignment Check */
    SetIDTGate((void*)Entry_AlignmentCheck_Handler, 0x11, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* Machine Check */
    SetIDTGate((void*)Entry_MachineCheck_Handler, 0x12, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* SIMD Floating-Point Exception */
    SetIDTGate((void*)Entry_SIMDFloatingPointException_Handler, 0x13, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* Virtualization Exception */
    SetIDTGate((void*)Entry_VirtualizationException_Handler, 0x14, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    
    /* Security Exception */
    SetIDTGate((void*)Entry_SecurityException_Handler, 0x1E, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* IRQs */
    SetIDTGate((void*)Entry_IRQ0_Handler, IRQ_START + 0, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ1_Handler, IRQ_START + 1, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ2_Handler, IRQ_START + 2, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ3_Handler, IRQ_START + 3, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ4_Handler, IRQ_START + 4, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ5_Handler, IRQ_START + 5, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ6_Handler, IRQ_START + 6, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ7_Handler, IRQ_START + 7, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ8_Handler, IRQ_START + 8, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ9_Handler, IRQ_START + 9, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ10_Handler, IRQ_START + 10, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ11_Handler, IRQ_START + 11, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ12_Handler, IRQ_START + 12, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ13_Handler, IRQ_START + 13, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ14_Handler, IRQ_START + 14, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ15_Handler, IRQ_START + 15, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    SetIDTGate((void*)Entry_IRQ16_Handler, IRQ_START + 16, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* APIC Timer */
    SetIDTGate((void*)Entry_LAPICTIMERInt_Handler, 0x30, InterruptGateType, KernelRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);
    
    /* Syscall */
    SetIDTGate((void*)Entry_SyscallInt_Handler, 0x80, InterruptGateType, UserAppRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    /* Sheduler call by app */
    SetIDTGate((void*)Entry_Schedule_Handler, 0x81, InterruptGateType, UserAppRing, GDTInfoSelectorsRing[KernelRing].Code, idtr);

    asm("lidt %0" : : "m" (idtr));     
}

extern "C" void DivideByZero_Handler(InterruptStack* Registers, uint64_t CoreID){
    Panic("Divide-by-zero Error Detected");
    globalLogs->Error("Divide-by-zero Error Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void Debug_Handler(InterruptStack* Registers, uint64_t CoreID){
    Panic("Debug Detected");
    globalLogs->Error("Debug Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void NMI_Handler(InterruptStack* Registers, uint64_t CoreID){
    Panic("Non-maskable Interrupt Detected");
    globalLogs->Error("Non-maskable Interrupt Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void Breakpoint_Handler(InterruptStack* Registers, uint64_t CoreID){
    Panic("Breakpoint Detected");
    globalLogs->Error("Breakpoint Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void Overflow_Handler(InterruptStack* Registers, uint64_t CoreID){
    Panic("Overflow Detected");
    globalLogs->Error("Overflow Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void BoundRangeExceeded_Handler(InterruptStack* Registers, uint64_t CoreID){
    Panic("Bound Range Exceeded Detected");
    globalLogs->Error("Bound Range Exceeded Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void InvalidOpcode_Handler(InterruptStack* Registers, uint64_t CoreID){
    Panic("Invalid Opcode Detected");
    globalLogs->Error("Invalid Opcode Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void DeviceNotAvailable_Handler(InterruptStack* Registers, uint64_t CoreID){
    Panic("Device Not Available Detected");
    globalLogs->Error("Device Not Available Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void DoubleFault_Handler(ErrorInterruptStack* Registers, uint64_t CoreID){
    Panic("Double Fault Detected");
    globalLogs->Error("Double Fault Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void InvalidTSS_Handler(ErrorInterruptStack* Registers, uint64_t CoreID){
    Panic("Invalid TSS Detected");
    globalLogs->Error("Invalid TSS Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void SegmentNotPresent_Handler(ErrorInterruptStack* Registers, uint64_t CoreID){
    Panic("Segment Not Present Detected");
    globalLogs->Error("Segment Not Present Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void StackSegmentFault_Handler(ErrorInterruptStack* Registers, uint64_t CoreID){
    Panic("Stack-Segment Fault Detected");
    globalLogs->Error("Stack-Segment Fault Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void GPFault_Handler(ErrorInterruptStack* Registers, uint64_t CoreID){
    Panic("General Protection Fault Detected");
    globalLogs->Error("General Protection Fault Detected -> At processor %u | RIP : %x", CoreID, Registers->rip);
    if(ReadBit((uint8_t)(uint64_t)Registers->errorCode, 0)){
        globalLogs->Message("The exception originated externally to the processor");
    }

    uint8_t Tbl = 0;
    Tbl |= ReadBit((uint8_t)(uint64_t)Registers->errorCode, 1);
    Tbl |= ReadBit((uint8_t)(uint64_t)Registers->errorCode, 2) << 1;
    switch (Tbl)
    {
    case 0:
        globalLogs->Message("Caused by gdt");
        break;
    case 1:
        globalLogs->Message("Caused by idt");
        break;
    case 2:
        globalLogs->Message("Caused by ldt");
        break;
    
    default:
        break;
    }
    while(true);
}

extern "C" void PageFault_Handler(ErrorInterruptStack* Registers, uint64_t CoreID, void* Address){
    globalLogs->Error("Page Fault Detected : Memory address : 0x%x | Processor ID : %u | RIP : %x", Address, CoreID, Registers->rip);
    Panic("Page Fault Detected");
    if(ReadBit((uint8_t)(uint64_t)Registers->errorCode, 0)){
        globalLogs->Message("Page-protection violation");
    }else{
        globalLogs->Message("Non-present page");
    }

    if(ReadBit((uint8_t)(uint64_t)Registers->errorCode, 1)){
        globalLogs->Message("Error caused by writting");
    }else{
        globalLogs->Message("Error caused by reading");
    }   

    if(ReadBit((uint8_t)(uint64_t)Registers->errorCode, 2)){
        globalLogs->Message("Non-user page");
    }

    if(ReadBit((uint8_t)(uint64_t)Registers->errorCode, 3)){
        globalLogs->Message("Page contain reserved bit");
    }

    if(ReadBit((uint8_t)(uint64_t)Registers->errorCode, 4)){
        globalLogs->Message("Caused by an instruction fetch");
    }

    if(ReadBit((uint8_t)(uint64_t)Registers->errorCode, 5)){
        globalLogs->Message("Protection-key violation");
    }

    if(ReadBit((uint8_t)(uint64_t)Registers->errorCode, 6)){
        globalLogs->Message("Caused by shadow stack access");
    }

    if(ReadBit((uint8_t)(uint64_t)Registers->errorCode, 7)){
        globalLogs->Message("Caused to an SGX violaton");
    }
    while(true);
}

extern "C" void x87FloatingPointException_Handler(InterruptStack* Registers, uint64_t CoreID){
    Panic("x87 Floating-Point Exception Detected");
    globalLogs->Error("x87 Floating-Point Exception Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void AlignmentCheck_Handler(ErrorInterruptStack* Registers, uint64_t CoreID){
    Panic("Alignment Check Detected");
    globalLogs->Error("Alignment Check Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void MachineCheck_Handler(InterruptStack* Registers, uint64_t CoreID){
    Panic("Machine Check Detected");
    globalLogs->Error("Machine Check Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void SIMDFloatingPointException_Handler(InterruptStack* Registers, uint64_t CoreID){
    Panic("SIMD Floating-Point Exception Detected");
    globalLogs->Error("SIMD Floating-Point Exception Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void VirtualizationException_Handler(InterruptStack* Registers, uint64_t CoreID){
    Panic("Virtualization Exception Detected");
    globalLogs->Error("Virtualization Exception Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void SecurityException_Handler(ErrorInterruptStack* Registers, uint64_t CoreID){
    Panic("Security Exception Detected");
    globalLogs->Error("Security Exception Detected -> At processor %u", CoreID);
    while(true);
}

extern "C" void IRQ0_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 0");
    APIC::localApicEOI();
}

extern "C" void IRQ1_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 1");
    uint8_t scancode = IoRead8(0x60);
    HandleKeyboard(scancode);
    APIC::localApicEOI();
}

extern "C" void IRQ2_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 2");
    APIC::localApicEOI();
}

extern "C" void IRQ3_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 3");
    APIC::localApicEOI();
}

extern "C" void IRQ4_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 4");
    APIC::localApicEOI();
}

extern "C" void IRQ5_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 5");
    APIC::localApicEOI();
}

extern "C" void IRQ6_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 6");
    APIC::localApicEOI();
}

extern "C" void IRQ7_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 7");
    APIC::localApicEOI();
}

extern "C" void IRQ8_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 8");
    APIC::localApicEOI();
}

extern "C" void IRQ9_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 9");
    APIC::localApicEOI();
}

extern "C" void IRQ10_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 10");
    APIC::localApicEOI();
}

extern "C" void IRQ11_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 11");
    APIC::localApicEOI();
}

extern "C" void IRQ12_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 12");
    uint8_t mousedata = IoRead8(0x60);
    HandlePS2Mouse(mousedata);
    APIC::localApicEOI();
}

extern "C" void IRQ13_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 13");
    APIC::localApicEOI();
}

extern "C" void IRQ14_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 14");
    APIC::localApicEOI();
}

extern "C" void IRQ15_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 15");
    APIC::localApicEOI();
}

extern "C" void IRQ16_Handler(InterruptStack* Registers){
    globalLogs->Warning("IRQ 16");
    APIC::localApicEOI();
}

extern "C" void LAPICTIMERInt_Handler(InterruptStack* Registers, uint64_t CoreID){
    globalTaskManager.Scheduler(Registers, CoreID);     
    APIC::localApicEOI();
}

extern "C" void Schedule_Handler(InterruptStack* Registers, uint64_t CoreID){
    globalTaskManager.Scheduler(Registers, CoreID);     
    APIC::localApicEOI();
}