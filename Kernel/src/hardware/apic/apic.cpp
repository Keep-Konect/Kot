#include "apic.h"

namespace APIC{ 
    LocalProcessor** Processor;
    uint8_t ProcessorCount;
    void* lapicAddressVirtual;

    void InitializeMADT(ACPI::MADTHeader* madt){
        lapicAddressVirtual = globalPageTableManager.MapMemory(0, 1);
        ProcessorCount = 0;
        Processor = (LocalProcessor**)malloc(sizeof(LocalProcessor) * MAX_PROCESSORS);
        if(madt == 0){
            return;
        }

        uint64_t entries = (madt->Header.Length - sizeof(ACPI::MADTHeader));
        
        for(uint64_t i = 0; i < entries;){
            EntryRecord* entryRecord = (EntryRecord*)((uint64_t)madt + sizeof(ACPI::MADTHeader) + i);
            i += entryRecord->Length;

            switch(entryRecord->Type){
                case EntryTypeLocalProcessor: {
                    LocalProcessor* processor = (LocalProcessor*)entryRecord;
                    Processor[ProcessorCount] = processor;
                    ProcessorCount++;        
                    break;                    
                }
                case EntryTypeIOAPIC:{
                    IOAPIC* ioApic = (IOAPIC*)entryRecord;
                    void* apicPtr = (void*)(uint64_t)ioApic->APICAddress;
                    break;
                }                    
                case EntryTypeInterruptSourceOverride:{
                    InterruptSourceOverride* iso = (InterruptSourceOverride*)entryRecord;
                    break;
                }                    
                case EntryTypeNonmaskableinterrupts:{
                    NonMaskableinterrupts* nmi = (NonMaskableinterrupts*)entryRecord;
                    break;
                }                    
                case EntryTypeLocalAPICAddressOverride:{
                    LocalAPICAddressOverride* local_apic_override = (LocalAPICAddressOverride*)entryRecord;
                    break;  
                }                                      
            }
        }
    }   

    void LoadCores(){
        uint64_t lapicAddress = (uint64_t)GetLAPICAddress();
        void* TrampolineVirtualAddress = globalPageTableManager.MapMemory((void*)0x8000, 1);

        uint8_t bspid = 0; 
        __asm__ __volatile__ ("mov $1, %%rax; cpuid; shrq $24, %%rbx;": "=r"(bspid)::);

        memcpy((void*)TrampolineVirtualAddress, (void*)&Trampoline, 0x1000);

        trampolineData* Data = (trampolineData*) (((uint64_t)&DataTrampoline - (uint64_t) &Trampoline) + TrampolineVirtualAddress);

        Data->MainEntry = (uint64_t)&TrampolineMain; 

        PageTable* PML4 = (PageTable*)globalAllocator.RequestPage();
        memset(globalPageTableManager.GetVirtualAddress(PML4), 0, 0x1000);
        
        //temp trampoline map
        globalPageTableManager.MapMemory((void*)0x8000, (void*)0x8000);

        for(int i = 1; i < ProcessorCount; i++){ 
            Data->Paging = (uint64_t)globalPageTableManager.PML4;
            uint64_t StackSize = 0x1000000; // 10 mb
            Data->Stack = (uint64_t)malloc(StackSize) + StackSize;
                
            if(Processor[i]->APICID == bspid) continue; 

            //init IPI
            localAPICWriteRegister(0x280, 0);
            localAPICWriteRegister(0x310, i << 24);
            localAPICWriteRegister(0x300, 0x00C500);
            do { __asm__ __volatile__ ("pause" : : : "memory"); }while(localAPICReadRegister(0x300) & (1 << 12));

            localAPICWriteRegister(0x310, i << 24);
            localAPICWriteRegister(0x300, 0x008500);
            do { __asm__ __volatile__ ("pause" : : : "memory"); }while(localAPICReadRegister(0x300) & (1 << 12));

            for(int j = 0; j < 2; j++) {

                localAPICWriteRegister(0x280, 0);
                localAPICWriteRegister(0x310, i << 24);
                localAPICWriteRegister(0x300, 0x000608);
                do { __asm__ __volatile__ ("pause" : : : "memory"); }while(localAPICReadRegister(0x300) & (1 << 12));
            }

            globalLogs->Warning("Wait processor %u", i);

            while (Data->Status != 3); // wait processor
            globalLogs->Successful("Processor %u respond with success", i);

            while (StatusProcessor != 4); // wait processor
            globalLogs->Successful("Processor %u is in the main function", i);
            StatusProcessor = 0;
        }
        globalPageTableManager.UnmapMemory((void*)0x8000);
    }  

    void* GetLAPICAddress(){
        void* lapicAddress = (void*)(msr::rdmsr(0x1b) & 0xfffff000);
        globalPageTableManager.MapMemory(lapicAddressVirtual, lapicAddress);
        return lapicAddressVirtual;
    }

    void EnableAPIC(){
        void* lapicAddress = (void*)(msr::rdmsr(0x1b) & 0xfffff000);
        msr::wrmsr(0x1b, (uint64_t)lapicAddress);
        localAPICWriteRegister(0xF0, localAPICReadRegister(0xF0) | 0x1ff);
    }

    static uint64_t mutexSLT;
    void StartLapicTimer(){
        Atomic::atomicSpinlock(&mutexSLT, 0);
        Atomic::atomicLock(&mutexSLT, 0);
        localApicEnableSpuriousInterrupts();
        // Setup Local APIC timer
        localAPICWriteRegister(LocalAPICRegisterOffsetInitialCount, 0x100000);
        uint32_t divide = localAPICReadRegister(LocalAPICRegisterOffsetDivide);
        localAPICWriteRegister(LocalAPICRegisterOffsetDivide, (divide & 0xfffffff4) | 0b1010);
        uint32_t timer = localAPICReadRegister(LocalAPICRegisterOffsetLVTTimer);

        LocalAPICInterruptRegister TimerRegisters;
        TimerRegisters.vector = 0x30;
        TimerRegisters.mask = LocalAPICInterruptRegisterMaskEnable;
        TimerRegisters.timerMode = LocalAPICInterruptTimerModePeriodic;
        
        localAPICWriteRegister(LocalAPICRegisterOffsetLVTTimer, CreatRegisterValueInterrupts(TimerRegisters) | (timer & 0xfffcef00));      
        Atomic::atomicUnlock(&mutexSLT, 0);
    }

    void localAPICSetTimerCount(uint32_t value){
        localAPICWriteRegister(LocalAPICRegisterOffsetInitialCount, value);
    }

    uint32_t localAPICGetTimerCount(){
        return localAPICReadRegister(LocalAPICRegisterOffsetCurentCount);
    }

    void localApicEOI(){        
        localAPICWriteRegister(LocalAPICRegisterOffsetEOI, 0);
    }

    void localApicEnableSpuriousInterrupts(){
        localAPICWriteRegister(LocalAPICRegisterOffsetSpuriouseIntVector, localAPICReadRegister(LocalAPICRegisterOffsetSpuriouseIntVector) | 0x100);
    }

    /* APIC */

    uint32_t localAPICReadRegister(size_t offset){
        void* lapicAddress = GetLAPICAddress();
	    return *((volatile uint32_t*)((void*)((uint64_t)lapicAddress + offset)));
    }

    uint32_t localAPICReadRegister(void* lapicAddress, size_t offset){
	    return *((volatile uint32_t*)((void*)((uint64_t)lapicAddress + offset)));
    }
    
    void localAPICWriteRegister(size_t offset, uint32_t value){
        void* lapicAddress = GetLAPICAddress();
        *((volatile uint32_t*)((void*)((uint64_t)lapicAddress + offset))) = value;
    }

    void localAPICWriteRegister(void* lapicAddress, size_t offset, uint32_t value){
        *((volatile uint32_t*)((void*)((uint64_t)lapicAddress + offset))) = value;
    }

    uint32_t CreatRegisterValueInterrupts(LocalAPICInterruptRegister reg){
	return (
		(reg.vector << LocalAPICInterruptVector) |
		(reg.messageType << LocalAPICInterruptMessageType) |
		(reg.deliveryStatus << LocalAPICInterruptDeliveryStatus) |
		(reg.triggerMode << LocalAPICInterruptTrigerMode) |
		(reg.mask << LocalAPICInterruptMask) |
		(reg.timerMode << LocalAPICInterruptTimerMode)
	);
}
}