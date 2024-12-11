//
// Created by Orange on 12/8/2024.
//

#include "Client.hpp"

#include <lazy_importer.hpp>
#include <xorstr.hpp>
#include <modules/DebugDetector.hpp>
#include <modules/SandboxDetector.hpp>
#include <CodeVirtualizer/VirtualizerSDK.h>
#include <modules/MemoryScanner.hpp>
#include <boost/thread/thread.hpp>
#include <thread>


namespace hdac::core
{
    Client::Client(): m_workGuard(m_ioContext.get_executor()), m_antiDebugTaskTimer(m_ioContext),
                      m_antiSandBoxTaskTimer(m_ioContext), m_memoryScannerTaskTimer(m_ioContext)
    {
    }

    __declspec(noinline)
    void Client::Run()
    {
        VIRTUALIZER_FALCON_TINY_START

        boost::thread_group threadpool;

        for (std::size_t i = 0; i < std::thread::hardware_concurrency(); i++)
            threadpool.create_thread([ObjectPtr = &m_ioContext] { return ObjectPtr->run(); });
        std::ignore = m_antiDebugTaskTimer.expiry();
        m_antiDebugTaskTimer.async_wait([this](const auto&)
        {
            AntiDebugTask();
        });

        std::ignore = m_antiSandBoxTaskTimer.expires_from_now();
        m_antiSandBoxTaskTimer.async_wait([this](const auto&)
        {
            AntiSandBoxTask();
        });


        std::ignore = m_memoryScannerTaskTimer.expires_from_now();
        m_memoryScannerTaskTimer.async_wait([this](const auto&)
        {

            MemoryScannerTask();
        });
        threadpool.join_all();
        VIRTUALIZER_FALCON_TINY_END;
    }

    __declspec(noinline)
    void Client::AntiDebugTask()
    {
        VIRTUALIZER_FALCON_TINY_START

        if (modules::DebugDetector::IsBeingDebugged())
            MessageBoxA(nullptr, xorstr_("Warning Debugger Detected!"), xorstr_("HD:AC"), MB_OK | MB_ICONWARNING);

        m_antiDebugTaskTimer.expires_after(std::chrono::seconds(5));
        m_antiDebugTaskTimer.async_wait([this](const auto& error)
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

        m_antiSandBoxTaskTimer.expires_after(std::chrono::seconds(5));
        m_antiSandBoxTaskTimer.async_wait([this](const auto& error)
        {
            if (!error) AntiSandBoxTask();
        });
        VIRTUALIZER_FALCON_TINY_END
    }

    void Client::MemoryScannerTask()
    {
        VIRTUALIZER_FALCON_TINY_START
        static modules::MemoryScanner scanner(m_ioContext);
        scanner.Scan();


        m_memoryScannerTaskTimer.expires_from_now(std::chrono::seconds(5));
        m_memoryScannerTaskTimer.async_wait([this](const auto&)
        {
            MemoryScannerTask();
        });
        VIRTUALIZER_FALCON_TINY_END
    }
}
