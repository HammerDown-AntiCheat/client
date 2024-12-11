//
// Created by Orange on 12/8/2024.
//
#pragma once
#include <boost/asio.hpp>

namespace hdac::core
{
    class Client
    {
    public:
        Client();

        void Run();

    private:
        void AntiDebugTask();
        void AntiSandBoxTask();
        void MemoryScannerTask();
        boost::asio::io_context m_ioContext;
        boost::asio::executor_work_guard<decltype(m_ioContext.get_executor())> m_workGuard;

        boost::asio::steady_timer m_antiDebugTaskTimer;
        boost::asio::steady_timer m_antiSandBoxTaskTimer;
        boost::asio::steady_timer m_memoryScannerTaskTimer;
    };
}
