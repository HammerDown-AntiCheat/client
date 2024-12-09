//
// Created by Orange on 12/8/2024.
//
#pragma once

namespace hdac::modules
{
    class SandboxDetector
    {
        public:
        [[nodiscard]]
        static bool IsInSandbox();
    };
}
