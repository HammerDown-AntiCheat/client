//
// Created by Orange on 12/8/2024.
//

#include "SandboxDetector.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <xorstr.hpp>
#include <lazy_importer.hpp>
#include <Windows.h>
#include <CodeVirtualizer/VirtualizerSDK.h>
namespace hdac::modules
{
    __declspec(noinline)
    bool SandboxDetector::IsInSandbox()
    {
        VIRTUALIZER_FALCON_TINY_START
        std::array<std::string, 12> blackListedModules =
        {
            xorstr_("SbieDll.dll"),
            xorstr_("avghooka.dll"),
            xorstr_("avghookx.dll"),
            xorstr_("snxhk.dll"),
            xorstr_("api_log.dll"),
            xorstr_("dbghelp.dll"),
            xorstr_("dir_watch.dll"),
            xorstr_("pstorec.dll"),
            xorstr_("vmcheck.dll"),
            xorstr_("wpespy.dll"),
            xorstr_("cmdvrt64.dll"),
            xorstr_("cmdvrt32.dll"),
        };

        const auto isInSandbox = std::ranges::any_of(blackListedModules, [](const auto& module)
        {
            return LI_FN(GetModuleHandleA)(module.c_str());
        });
        VIRTUALIZER_FALCON_TINY_END;

        return isInSandbox;
    }
}
