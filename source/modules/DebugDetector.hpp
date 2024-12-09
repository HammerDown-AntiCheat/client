//
// Created by Orange on 12/8/2024.
//
#pragma once
#include <cstdint>


namespace hdac::modules
{
    class DebugDetector
    {
    public:
        [[nodiscard]]
        static bool IsBeingDebugged();
    private:
        __declspec(noinline)
        [[nodiscard]]
        static bool IsDebuggerPresentCheck();

        __declspec(noinline)
        [[nodiscard]]
        static bool IsRemoteDebuggerPresentCheck();

        __declspec(noinline)
        [[nodiscard]]
        static bool IsPebDebugFlag();

        __declspec(noinline)
        [[nodiscard]]
        static bool IsNtFlagsDebug();

        __declspec(noinline)
        [[nodiscard]]
        static bool IsHeapDebugFlags();

        __declspec(noinline)
        [[nodiscard]]
        static uint32_t GetHeapFlags();

        __declspec(noinline)
        [[nodiscard]]
        static bool IsNtQueryInformationProcessProcessDebugPort();

        __declspec(noinline)
        [[nodiscard]]
        static bool IsNtQueryInformationProcessProcessDebugFlags();

        __declspec(noinline)
        [[nodiscard]]
        static bool IsHasHardwareBreakPoints();
    };
} // modules
