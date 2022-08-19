#include <kot/sys.h>
#include <kot/heap.h>
#include <kot/memory.h>
#include <core/main.h>

process_t proc;

void ShareString(thread_t self, char* str, uint64_t* clientAddress){
    SYS_ShareDataUsingStackSpace(self, (uint64_t)str, strlen(str) + 1, clientAddress);
}

extern "C" int main(struct KernelInfo* kernelInfo) {

    Printlog("[System] Initialization ...");

    Sys_GetthreadKey(&proc);
    
    thread_t self;
    Sys_GetthreadKey(&self);

    ramfs::Parse(kernelInfo->ramfs.address, kernelInfo->ramfs.size);

    // load IPC
    KotSpecificData.UISDHandler = UISDInitialize(&KotSpecificData.UISDHandlerProcess);
    
    // load start file
    ramfs::File* InitFile = ramfs::Find("Starter.cfg");
    // todo delete testClass
    ramfs::File* testClass = ramfs::Find("Test.class");

    if (InitFile != NULL) {

        // fetch services

        uintptr_t testClassBuffer = (uintptr_t) calloc(testClass->size + 1);
        ramfs::Read(testClass, testClassBuffer);

        char* BufferInitFile = (char*) calloc(InitFile->size + 1);
        ramfs::Read(InitFile, BufferInitFile);
        BufferInitFile[InitFile->size + 1] = NULL;
        char* app;
        char** ServicesInfo = strsplit(BufferInitFile, "\n");

        arguments_t* InitParameters = (arguments_t*)calloc(sizeof(arguments_t));

        for (uint64_t i = 0; ServicesInfo[i] != NULL; i++) {
            char** ServiceInfo = strsplit(ServicesInfo[i], ", ");
            ramfs::File* ServiceFile = ramfs::Find(ServiceInfo[0]);
            if (ServiceFile != NULL && atoi(ServiceInfo[1]) >= PriviledgeDriver && atoi(ServiceInfo[1]) <= PriviledgeApp) {
                
                // load service in a thread

                uintptr_t BufferServiceFile = calloc(ServiceFile->size);
                ramfs::Read(ServiceFile, BufferServiceFile);
                thread_t thread = NULL;
                ELF::loadElf(BufferServiceFile, (enum Priviledge) atoi(ServiceInfo[1]), NULL, &thread);
                free(BufferServiceFile);

                char** Parameters = (char**) calloc(sizeof(char*));

                InitParameters->arg[0] = 1;
                ShareString(thread, ServiceInfo[0], (uint64_t*) &Parameters[0]);
                SYS_ShareDataUsingStackSpace(thread, (uint64_t) Parameters, sizeof(char*), &InitParameters->arg[1]);
                SYS_ShareDataUsingStackSpace(thread, (uint64_t) &kernelInfo->framebuffer, sizeof(framebuffer_t), &InitParameters->arg[2]);
                SYS_ShareDataUsingStackSpace(thread, (uint64_t) testClassBuffer, testClass->size + 1, &InitParameters->arg[3]);
    
                Sys_Execthread(thread, InitParameters, ExecutionTypeQueu, NULL);
            
            }     
            freeSplit(ServiceInfo);
        }
        freeSplit(ServicesInfo);
        free(InitParameters);
    } else {
        Printlog("[System] Starter.cfg not found");
        return KFAIL;
    }

    Printlog("[System] All tasks in 'Starter.cfg' are loaded");
    Printlog("[System] Service initialized successfully");

    return KSUCCESS;

}