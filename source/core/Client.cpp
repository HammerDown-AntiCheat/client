//
// Created by Orange on 12/8/2024.
//

#include "Client.hpp"

#include <lazy_importer.hpp>
#include <xorstr.hpp>
#include <modules/DebugDetector.hpp>
#include <modules/SandboxDetector.hpp>
#include <CodeVirtualizer/VirtualizerSDK.h>


namespace hdac::core
{
    Client::Client(): m_workGuard(m_ioContext.get_executor()), m_antiDebugTaskTimer(m_ioContext),
                      m_antiSandBoxTaskTimer(m_ioContext)
    {
    }

    __declspec(noinline)
    void Client::Run()
    {
        VIRTUALIZER_FALCON_TINY_START
        boost::asio::post([this]()
        {
            AntiDebugTask();
        });

        boost::asio::post([this]()
        {
            AntiSandBoxTask();
        });

        m_ioContext.run();
        VIRTUALIZER_FALCON_TINY_END
    }

    __declspec(noinline)
    void Client::AntiDebugTask()
    {
        VIRTUALIZER_FALCON_TINY_START
        if (modules::DebugDetector::IsBeingDebugged())
            MessageBoxA(nullptr, xorstr_("Warning Debugger Detected!"), xorstr_("HD:AC"), MB_OK | MB_ICONWARNING);

        m_antiDebugTaskTimer.expires_after(std::chrono::seconds(1));
        m_antiDebugTaskTimer.async_wait([this](const boost::system::error_code& error)
        {
            if (!error) AntiDebugTask();
        });

        VIRTUALIZER_FALCON_TINY_END
    }

    void Client::AntiSandBoxTask()
    {
        VIRTUALIZER_FALCON_TINY_START

        if (modules::SandboxDetector::IsInSandbox())
            MessageBoxA(nullptr, xorstr_("Warning Sandbox detected!"), xorstr_("HD:AC"), MB_OK | MB_ICONWARNING);

        m_antiSandBoxTaskTimer.expires_after(std::chrono::seconds(1));
        m_antiSandBoxTaskTimer.async_wait([this](const boost::system::error_code& error)
        {
            if (!error) AntiSandBoxTask();
        });
        VIRTUALIZER_FALCON_TINY_END
    }
}
