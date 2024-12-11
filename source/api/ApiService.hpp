//
// Created by Orange on 12/10/2024.
//
#pragma once
#include <vector>
#include <mutex>
#include <string>

namespace hdac::api
{

    struct DetectInfo
    {
        std::string cheatName;
        std::string method;
    };
    class ApiService
    {
        std::string_view url = "http://localhost/detect";
        mutable std::mutex m_mutex;
    public:
        std::optional<DetectInfo> SendPageToAnalysis(const std::vector<uint8_t>& page) const;


    };
} // api
