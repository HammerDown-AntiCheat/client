//
// Created by Orange on 12/9/2024.
//
#pragma once
#include <compare>
#include <cstdint>
#include <set>
#include <vector>

namespace hdac::modules
{
    struct MemoryPage
    {
        uintptr_t begin;
        uintptr_t end;

        auto operator<=>(const MemoryPage&) const = default;

        [[nodiscard]] std::vector<uint8_t> GetData() const;
    };

    class MemoryScanner
    {
    public:
        void Scan();
    private:
        std::set<MemoryPage> m_checkedPages;

        [[nodiscard]]
        static std::vector<MemoryPage> GetExecutableMemoryRegions();

    };
} // modules