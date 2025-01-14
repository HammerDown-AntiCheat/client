//
// Created by Orange on 12/9/2024.
//
#pragma once
#include <compare>
#include <cstdint>
#include <unordered_set>
#include <vector>
#include <cstddef>
#include <string>
#include <expected>
#include <api/ApiService.hpp>
#include <boost/asio/io_service.hpp>

struct MemoryPage
{
    uintptr_t begin;
    uintptr_t end;

    auto operator<=>(const MemoryPage&) const = default;

    [[nodiscard]] std::vector<uint8_t> GetData() const;

    [[nodiscard]] size_t Size() const;

    [[nodiscard]] uint32_t Crc32() const;
};

template<>
struct std::hash<MemoryPage>
{
    size_t operator()(const MemoryPage& page) const noexcept
    {
        constexpr std::hash<uintptr_t> hasher;
        return hasher(page.begin) ^ hasher(page.end);
    }
};


namespace hdac::modules
{
    class MemoryScanner
    {
    public:
        explicit MemoryScanner(boost::asio::io_context& io_service);

        void Scan();

    private:
        boost::asio::io_context& m_ioContext;

        std::unordered_set<uint32_t> m_checkedPages;

        std::shared_mutex m_mutex;
        api::ApiService m_apiService;
        [[nodiscard]]
        static std::vector<MemoryPage> GetExecutableMemoryRegions();
    };
} // modules

