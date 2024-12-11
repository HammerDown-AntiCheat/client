//
// Created by Orange on 12/8/2024.
//
#include <Windows.h>
#include <core/Client.hpp>
#include <print>

#ifdef HDAC_AS_DL
extern "C" BOOL WINAPI DllMain(const HMODULE dllHandle, const DWORD callReason, LPVOID)
{
    if (callReason != DLL_PROCESS_ATTACH)
        return false;

    DisableThreadLibraryCalls(dllHandle);
    std::thread([] {hdac::core::Client().Run();}).detach();

    return true;
}
#else
int main()
{
    hdac::core::Client client;
    client.Run();
    std::print("Finished!\n");
}
#endif