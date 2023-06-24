#include <system/system.h>

KResult SetupStack(void** Data, size64_t* Size, int argc, char** argv, char** envp){
    size64_t args = 0;
    for(int i = 0; i < argc; i++){
        args += strlen(argv[i]) + 1; // Add NULL char at the end
    }
    size64_t envc = 0;
    size64_t envs = 0;
    auto ev = envp;
	while(*ev){
		envc++;
        envs += strlen(*ev) + 1; // Add NULL char at the end
	}

    *Size = sizeof(void*) + (argc + 1) * sizeof(char*) + (envc + 1) * sizeof(char*) + args + envs;
    void* Buffer = malloc(*Size);
    
    void* StackDst = Buffer;

    *(void**)StackDst = (void*)argc;
    StackDst = (void*)((uint64_t)StackDst + sizeof(void*));

    void* OffsetDst = StackDst;
    StackDst = (void*)((uint64_t)StackDst + (argc + 1) * sizeof(char*) + (envc + 1) * sizeof(char*));

    for(int i = 0; i < argc; i++){
        *((void**)OffsetDst) = (void*)((uint64_t)StackDst - (uint64_t)Buffer);
        OffsetDst = (void*)((uint64_t)OffsetDst + sizeof(void*));
        strcpy((char*)StackDst, argv[i]);
        StackDst = (void*)((uint64_t)StackDst + strlen(argv[i]) + 1); // Add NULL char at the end
    }

    // Null argument
    *(void**)OffsetDst = NULL;
    OffsetDst = (void*)((uint64_t)OffsetDst + sizeof(void*));

    for(int i = 0; i < envc; i++){
        *(void**)OffsetDst = (void*)((uint64_t)StackDst - (uint64_t)Buffer);
        OffsetDst = (void*)((uint64_t)OffsetDst + sizeof(void*));
        strcpy((char*)StackDst, envp[i]);
        StackDst = (void*)((uint64_t)StackDst + strlen(envp[i]) + 1); // Add NULL char at the end
    }
    // Null argument
    *(void**)OffsetDst = NULL;

    *Data = Buffer;

    return KSUCCESS;
}

KResult ExecuteSystemAction(uint64_t PartitonID){
    // Load filesystem handler
    if(!KotSpecificData.VFSHandler){
        kot_srv_storage_callback_t* Callback = kot_Srv_Storage_VFSLoginApp(kot_Sys_GetProcess(), FS_AUTHORIZATION_HIGH, Storage_Permissions_Admin | Storage_Permissions_Read | Storage_Permissions_Write | Storage_Permissions_Create, "/d0:/", true);
        KotSpecificData.VFSHandler = Callback->Data;
        free(Callback);
    }

    // TODO certify disk as system
    bool IsSystem = true;
    if(IsSystem){
        KResult Status = KFAIL;

        char DiskNumberBuffer[20];
        char* DiskNumber = itoa(PartitonID, (char*)&DiskNumberBuffer, 10);

        StringBuilder* SystemDataPathBuilder = new StringBuilder("/d");
        SystemDataPathBuilder->append(DiskNumberBuffer);
        SystemDataPathBuilder->append(":/kot/System/Starter.json");
        char* SystemDataPath = SystemDataPathBuilder->toString();
        delete SystemDataPathBuilder;
        FILE* SystemDataFile = fopen(SystemDataPath, "r");

        if(SystemDataFile){
            fseek(SystemDataFile, 0, SEEK_END);
            size_t SystemDataFileSize = ftell(SystemDataFile);
            fseek(SystemDataFile, 0, SEEK_SET);

            char* BufferSystemDataFile = (char*)malloc(SystemDataFileSize);
            fread(BufferSystemDataFile, SystemDataFileSize, 1, SystemDataFile);

            JsonParser* Parser = new JsonParser(BufferSystemDataFile);


            if(Parser->getCode() == JSON_SUCCESS && Parser->getValue()->getType() == JSON_ARRAY){
                JsonArray* Array = (JsonArray*) Parser->getValue();

                kot_arguments_t* InitParameters = (kot_arguments_t*)calloc(1, sizeof(kot_arguments_t));


                for(uint64_t i = 0; i < Array->length(); i++){
                    JsonObject* Service = (JsonObject*) Array->Get(i);
                    JsonString* File = (JsonString*) Service->Get("file");
                    JsonNumber* Priviledge = (JsonNumber*) Service->Get("priviledge"); // default: 3
                    JsonNumber* FlagsJson = (JsonNumber*) Service->Get("flags"); // launch flags 
                    
                    if (File->getType() == JSON_STRING) {
                        if(!strcmp(File->Get(), "")) continue;
                        int32_t ServicePriledge = 3;
                        if(Priviledge != NULL){
                            if(Priviledge->getType() == JSON_NUMBER){ 
                                if(Priviledge->Get() >= 1 && Priviledge->Get() <= 3){
                                    ServicePriledge = Priviledge->Get();
                                }
                            }
                        }
                        uint64_t Flags = 0;
                        if(FlagsJson != NULL){
                            if(FlagsJson->getType() == JSON_NUMBER){ 
                                Flags = FlagsJson->Get();
                            }
                        }

                        StringBuilder* ServicePathBuilder = new StringBuilder("/d");
                        ServicePathBuilder->append(DiskNumberBuffer);
                        ServicePathBuilder->append(":/");
                        ServicePathBuilder->append(File->Get());
                        char* FilePath = ServicePathBuilder->toString();
                        delete ServicePathBuilder;

                        kot_srv_system_callback_t* Callback = kot_Srv_System_LoadExecutable(Priviledge->Get(), FilePath, true);

                        if(Callback->Status == KSUCCESS){
                            void* MainStackData;
                            size64_t SizeMainStackData;
                            char* Argv[] = {FilePath, NULL};
                            char* Env[] = {NULL};
                            SetupStack(&MainStackData, &SizeMainStackData, 1, Argv, Env);
                            free(FilePath);

                            kot_ShareDataWithArguments_t Data{
                                .Data = MainStackData,
                                .Size = SizeMainStackData,
                                .ParameterPosition = 0x0,
                            };
                            InitParameters->arg[2] = Flags;

                            assert(kot_Sys_ExecThread((kot_thread_t)Callback->Data, InitParameters, ExecutionTypeQueu, &Data) == KSUCCESS);
                            free(MainStackData);
                            free(Callback);
                        }else{
                            free(FilePath);
                            free(Callback);
                        }
                    }
                }
                free(InitParameters);
                Status = KSUCCESS;
            }
            delete Parser;
            free(SystemDataFile);
            free(BufferSystemDataFile);
        }
        free(SystemDataPath);

        return Status;
    }

    return KNOTALLOW;
}