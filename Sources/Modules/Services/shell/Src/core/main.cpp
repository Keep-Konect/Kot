#include "main.h"

extern "C" int main() {

    process_t self = Sys_GetProcess();

    uint32_t wid = orb::Create(300, 300, 10, 10);
    uintptr_t fb = orb::GetFramebuffer(wid);

}