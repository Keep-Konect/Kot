#include <main/main.h>

uintptr_t TableConverter;
size64_t TableConverterCharCount;

extern "C" int main(){
    GetTableConverter("d0:azerty.bin", &TableConverter, &TableConverterCharCount);

    SrvInitalize();

    return KSUCCESS;
}