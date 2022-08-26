#include <kot/uisd/srvs/system.h>

thread_t srv_system_callback_thread = NULL;
uisd_system_t* SystemData = NULL;

void Srv_System_Initialize(){
    SystemData = (uisd_system_t*)FindControllerUISD(ControllerTypeEnum_System);
    if(SystemData != NULL){
        process_t Proc = NULL;
        Sys_GetProcessKey(&Proc);

        thread_t SystemthreadKeyCallback = NULL;
        Sys_Createthread(Proc, &Srv_System_Callback, PriviledgeApp, &SystemthreadKeyCallback);
        srv_system_callback_thread = MakeShareableThreadToProcess(SystemthreadKeyCallback, SystemData->ControllerHeader.Process);
    }else{
        Sys_Close(KFAIL);
    }
}

void Srv_System_Callback(KResult Statu, struct srv_system_callback_t* Callback, uint64_t GP0, uint64_t GP1, uint64_t GP2, uint64_t GP3){
    Callback->Statu = Callback->Handler(Statu, Callback, GP0, GP1, GP2, GP3);
    if(Callback->IsAwait){
        Sys_Unpause(Callback->Self);
    }
    Sys_Close(KSUCCESS);
}

/* GetFrameBufer */
KResult Srv_System_GetFrameBufer_Callback(KResult Statu, struct srv_system_callback_t* Callback, uint64_t GP0, uint64_t GP1, uint64_t GP2, uint64_t GP3){
    if(Statu == KSUCCESS){
        memcpy(Callback->Data, (uintptr_t)GP0, sizeof(srv_system_framebuffer_t));
        Callback->Size = (size64_t)sizeof(srv_system_framebuffer_t);
    }
    return Statu;
}

struct srv_system_callback_t* Srv_System_GetFrameBufer(srv_system_framebuffer_t* framebuffer, bool IsAwait){
    if(!srv_system_callback_thread) Srv_System_Initialize();
    
    thread_t self;
    Sys_GetthreadKey(&self);

    struct srv_system_callback_t* callback = (struct srv_system_callback_t*)malloc(sizeof(struct srv_system_callback_t));
    callback->Self = self;
    callback->Data = framebuffer;
    callback->IsAwait = IsAwait;
    callback->Statu = KBUSY;
    callback->Handler = &Srv_System_GetFrameBufer_Callback;

    struct arguments_t parameters;
    parameters.arg[0] = srv_system_callback_thread;
    parameters.arg[1] = callback;
    

    KResult statu = Sys_Execthread(SystemData->GetFramebuffer, &parameters, ExecutionTypeQueu, NULL);
    if(statu == KSUCCESS && IsAwait){
        Sys_Pause(false);
    }
    return callback;
}

/* ReadFile */
KResult Srv_System_ReadFileInitrd_Callback(KResult Statu, struct srv_system_callback_t* Callback, uint64_t GP0, uint64_t GP1, uint64_t GP2, uint64_t GP3){
    if(Statu == KSUCCESS){
        Callback->Data = malloc((size64_t)GP0);
        memcpy(Callback->Data, (uintptr_t)GP0, (size64_t)GP0);
        Callback->Size = (size64_t)GP0;
    }
    return Statu;
}

struct srv_system_callback_t* Srv_System_ReadFileInitrd(char* Name,  bool IsAwait){
    if(!srv_system_callback_thread) Srv_System_Initialize();
    
    thread_t self;
    Sys_GetthreadKey(&self);

    struct srv_system_callback_t* callback = (struct srv_system_callback_t*)malloc(sizeof(struct srv_system_callback_t));
    callback->Self = self;
    callback->Data = NULL;
    callback->IsAwait = IsAwait;
    callback->Statu = KBUSY;
    callback->Handler = &Srv_System_ReadFileInitrd_Callback;

    struct ShareDataWithArguments_t data;
    data.Data = Name;
    data.Size = strlen(Name + 1); // add '\0' char
    data.ParameterPosition = 0x2; 

    struct arguments_t parameters;
    parameters.arg[0] = srv_system_callback_thread;
    parameters.arg[1] = callback;
    

    KResult statu = Sys_Execthread(SystemData->GetFramebuffer, &parameters, ExecutionTypeQueu, &data);
    if(statu == KSUCCESS && IsAwait){
        Sys_Pause(false);
    }
    return callback;
}