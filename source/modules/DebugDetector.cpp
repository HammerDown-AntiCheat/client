//
// Created by Orange on 12/8/2024.
//

#include "DebugDetector.hpp"

#include <algorithm>
#include <array>
#include <CodeVirtualizer/VirtualizerSDK.h>
#include <lazy_importer.hpp>
#include <Windows.h>
#include <winternl.h>

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENV64BIT
#else
    #define ENV32BIT
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
    #define ENV64BIT
#else
    #define ENV32BIT
#endif
#endif


namespace hdac::modules
{
#pragma optimize( "", off )
    __declspec(noinline)
    bool DebugDetector::IsBeingDebugged()
    {
        constexpr std::array checks =
        {
            IsDebuggerPresentCheck,
            IsRemoteDebuggerPresentCheck,
            IsPebDebugFlag,
            IsNtFlagsDebug,
            IsHeapDebugFlags,
            IsNtQueryInformationProcessProcessDebugPort,
            IsNtQueryInformationProcessProcessDebugFlags,
            IsHasHardwareBreakPoints
        };
        VIRTUALIZER_FALCON_TINY_START

        const auto status = std::ranges::any_of(checks, [](auto check)
        {
            return check();
        });
        VIRTUALIZER_FALCON_TINY_END
        return status;
    }
#pragma optimize( "", on)
    bool DebugDetector::IsDebuggerPresentCheck()
    {
        VIRTUALIZER_FALCON_TINY_START
        bool debugged = false;
        debugged = LI_FN(IsDebuggerPresent)();
        VIRTUALIZER_FALCON_TINY_END

        return debugged;
    }

    bool DebugDetector::IsRemoteDebuggerPresentCheck()
    {
        VIRTUALIZER_FALCON_TINY_START
        BOOL beingDebugged = FALSE;

        LI_FN(CheckRemoteDebuggerPresent)(LI_FN(GetCurrentProcess)(), &beingDebugged);

        VIRTUALIZER_FALCON_TINY_END
        return beingDebugged;
    }

    bool DebugDetector::IsPebDebugFlag()
    {
#ifdef ENV64BIT
        const auto peb = reinterpret_cast<PPEB>(__readgsqword(0x60));
#elif ENV32BIT
        const auto peb = reinterpret_cast<PPEB>(__readgsqword(0x30));
#endif
        return peb->BeingDebugged == 1;
    }
#pragma optimize( "", off)
    bool DebugDetector::IsNtFlagsDebug()
    {
        VIRTUALIZER_FALCON_TINY_START
        PDWORD pNtGlobalFlag = nullptr, pNtGlobalFlagWoW64 = nullptr;
#ifdef ENV64BIT
        pNtGlobalFlag = reinterpret_cast<PDWORD>(__readgsqword(0x60) + 0xBC);

#elif ENV32BIT
        BYTE* _teb32 = (BYTE*)__readfsdword(0x18);
        DWORD _peb32 = *(DWORD*)(_teb32 + 0x30);
        pNtGlobalFlag = (PDWORD)(_peb32 + 0x68);

        BYTE* _teb64 = (BYTE*)__readfsdword(0x18) - 0x2000;
        DWORD64 _peb64 = *(DWORD64*)(_teb64 + 0x60);
        pNtGlobalFlagWoW64 = (PDWORD)(_peb64 + 0xBC);

#endif
        const bool normalDetected = pNtGlobalFlag && *pNtGlobalFlag & 0x00000070;
        const bool wow64Detected = pNtGlobalFlagWoW64 && *pNtGlobalFlagWoW64 & 0x00000070;


        const auto status = normalDetected || wow64Detected;
        VIRTUALIZER_FALCON_TINY_END
        return status;
    }
#pragma optimize( "", on)
    bool DebugDetector::IsHeapDebugFlags()
    {
        return GetHeapFlags() > 2;
    }

    uint32_t DebugDetector::GetHeapFlags()
    {
        VIRTUALIZER_FALCON_TINY_START
#ifdef ENV64BIT
        const auto pProcessHeap = reinterpret_cast<PINT64>(__readgsqword(0x60) + 0x30);
        const auto pHeapFlags = reinterpret_cast<PUINT32>(*pProcessHeap + 0x70);
#elifdef ENV32BIT
        const auto pProcessHeap = reinterpret_cast<PINT32>(__readfsdword(0x30) + 0x18);
        const auto pHeapFlags = reinterpret_cast<PUINT32>(*pProcessHeap + 0x40);
#endif
        VIRTUALIZER_FALCON_TINY_END
        return *pHeapFlags;
    }

    bool DebugDetector::IsNtQueryInformationProcessProcessDebugPort()
    {
        // ProcessDebugPort
        constexpr int ProcessDbgPort = 7;
        VIRTUALIZER_FALCON_TINY_START

#ifdef ENV64BIT
        constexpr DWORD dProcessInformationLength = sizeof(ULONG) * 2;
        DWORD64 IsRemotePresent = 0;

#elifdef ENV32BIT
        constexpr DWORD dProcessInformationLength = sizeof(ULONG);
        DWORD32 IsRemotePresent = 0;
#endif

        const auto func = LI_FN(NtQueryInformationProcess).get();
        const auto status = func(LI_FN(GetCurrentProcess)(), static_cast<PROCESSINFOCLASS>(ProcessDbgPort),
                                 &IsRemotePresent,
                                 dProcessInformationLength, nullptr);
        VIRTUALIZER_FALCON_TINY_END
        return status == 0 && IsRemotePresent != 0;
    }

    bool DebugDetector::IsNtQueryInformationProcessProcessDebugFlags()
    {
        constexpr int ProcessDebugFlags = 0x1f;
        VIRTUALIZER_FALCON_TINY_START
        const auto func = LI_FN(NtQueryInformationProcess).get();
        DWORD NoDebugInherit = 0;

        const auto status = func(LI_FN(GetCurrentProcess)(), static_cast<PROCESSINFOCLASS>(ProcessDebugFlags),
                                 &NoDebugInherit, sizeof(DWORD), nullptr);
        VIRTUALIZER_FALCON_TINY_END
        return status == 0 && NoDebugInherit == 0;
    }

    bool DebugDetector::IsHasHardwareBreakPoints()
    {
        VIRTUALIZER_FALCON_TINY_START
        CONTEXT ctx{};


        ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

        if (LI_FN(GetThreadContext)(LI_FN(GetCurrentThread)(), &ctx))
            return ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0;

        VIRTUALIZER_FALCON_TINY_END
        return true;
    }
}
