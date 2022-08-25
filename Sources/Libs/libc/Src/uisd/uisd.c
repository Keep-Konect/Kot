#include <kot/uisd.h>

uintptr_t ControllerList[ControllerCount];

size64_t ControllerTypeSize[ControllerCount] = {
    sizeof(uisd_system_t),
    sizeof(uisd_graphics_t),
    sizeof(uisd_audio_t),
    sizeof(uisd_storage_t),
    sizeof(uisd_vfs_t),
    sizeof(uisd_usb_t),
    sizeof(uisd_pci_t)
};

thread_t CallBackUISDThread = NULL;
process_t ProcessKeyForUISD = NULL;

KResult CallbackUISD(uint64_t Task, KResult Statu, uisd_callbackInfo_t* Info, uint64_t GP0, uint64_t GP1);

KResult InitializeUISD(){
    thread_t UISDthreadKeyCallback;
    uint64_t UISDKeyFlags = NULL;
    Keyhole_SetFlag(&UISDKeyFlags, KeyholeFlagPresent, true);
    Keyhole_SetFlag(&UISDKeyFlags, KeyholeFlagDataTypethreadIsExecutableWithQueue, true);

    process_t Proc = NULL;
    Sys_GetProcessKey(&Proc);

    UISDKeyFlags = NULL;
    Sys_Createthread(Proc, &CallbackUISD, PriviledgeApp, &UISDthreadKeyCallback);
    Sys_Keyhole_CloneModify(UISDthreadKeyCallback, &CallBackUISDThread, KotSpecificData.UISDHandlerProcess, UISDKeyFlags, PriviledgeApp);

    UISDKeyFlags = NULL;
    Keyhole_SetFlag(&UISDKeyFlags, KeyholeFlagPresent, true);
    Keyhole_SetFlag(&UISDKeyFlags, KeyholeFlagDataTypeProcessMemoryAccessible, true);
    Sys_Keyhole_CloneModify(Proc, &ProcessKeyForUISD, KotSpecificData.UISDHandlerProcess, UISDKeyFlags, PriviledgeApp);
    
    meset(&ControllerList, NULL, ControllerCount * sizeof(uinptr_t));
    
    return KSUCCESS;
}

KResult CallbackUISD(uint64_t Task, KResult Statu, uisd_callbackInfo_t* Info, uint64_t GP0, uint64_t GP1){
    if(Task == UISDGetTask){
        ControllerList[Info->Controller] = (uintptr_t)GP0;
        Info->Location = (uintptr_t)GP0;
    } 
    Info->Statu = Statu;
    if(Info->AwaitCallback){
        Sys_Unpause(Info->Self);
    }
    Sys_Close(KSUCCESS);
}

uisd_callbackInfo_t* GetControllerUISD(enum ControllerTypeEnum Controller, uintptr_t* Location, bool AwaitCallback){
    if(!CallBackUISDThread) InitializeUISD();
    thread_t Self = NULL;
    Sys_GetthreadKey(&Self);
    uisd_callbackInfo_t* Info = (uisd_callbackInfo_t*)malloc(sizeof(uisd_callbackInfo_t));
    Info->Self = Self;
    Info->Controller = Controller;
    Info->AwaitCallback = AwaitCallback;
    Info->Location = NULL;
    Info->Statu = KFAIL;

    struct arguments_t parameters;
    parameters.arg[0] = UISDGetTask;
    parameters.arg[1] = Controller;
    parameters.arg[2] = CallBackUISDThread;
    parameters.arg[3] = Info;
    parameters.arg[4] = ProcessKeyForUISD;
    parameters.arg[5] = (uint64_t)*Location;
    Sys_Execthread(KotSpecificData.UISDHandler, &parameters, ExecutionTypeQueu, NULL);
    if(AwaitCallback){
        Sys_Pause(false);
        *Location = Info->Location;
        return Info;
    }
    return Info;
}

uisd_callbackInfo_t* CreateControllerUISD(enum ControllerTypeEnum Controller, ksmem_t MemoryField, bool AwaitCallback){
    if(!CallBackUISDThread) InitializeUISD();
    thread_t Self = NULL;
    Sys_GetthreadKey(&Self);
    uisd_callbackInfo_t* Info = malloc(sizeof(uisd_callbackInfo_t));
    Info->Self = Self;
    Info->Controller = Controller;
    Info->AwaitCallback = AwaitCallback;
    Info->Statu = KFAIL;

    uint64_t Flags = NULL;
    Keyhole_SetFlag(&Flags, KeyholeFlagPresent, true);
    ksmem_t MemoryFieldKey = NULL;
    Sys_Keyhole_CloneModify(MemoryField, &MemoryFieldKey, KotSpecificData.UISDHandlerProcess, Flags, PriviledgeApp);

    struct arguments_t parameters;
    parameters.arg[0] = UISDCreateTask;
    parameters.arg[1] = Controller;
    parameters.arg[2] = CallBackUISDThread;
    parameters.arg[3] = Info;
    parameters.arg[4] = MemoryFieldKey;
    Sys_Execthread(KotSpecificData.UISDHandler, &parameters, ExecutionTypeQueu, NULL);
    if(AwaitCallback){
        Sys_Pause(false);
        return Info;
    }
    return Info;
}

/* Useful functions */

thread_t MakeThreadShareable(thread_t Thread, enum Priviledge priviledgeRequired){
    thread_t ReturnValue;
    uint64_t UISDKeyFlags = NULL;
    Keyhole_SetFlag(&UISDKeyFlags, KeyholeFlagPresent, true);
    Keyhole_SetFlag(&UISDKeyFlags, KeyholeFlagDataTypethreadIsExecutableWithQueue, true);
    Sys_Keyhole_CloneModify(Thread, &ReturnValue, NULL, UISDKeyFlags, PriviledgeApp);
    return ReturnValue;
}

uinptr_t GetControllerLocationUISD(enum ControllerTypeEnum Controller){
    return ControllerList[Controller];
}

uinptr_t FindControllerUISD(enum ControllerTypeEnum Controller){
    uinptr_t ControllerData = GetControllerLocationUISD(Controller);
    if(!ControllerData){
        
    }
    return ControllerData;
}

