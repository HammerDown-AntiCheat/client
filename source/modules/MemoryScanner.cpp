//
// Created by Orange on 12/9/2024.
//

#include "MemoryScanner.hpp"
#include <CodeVirtualizer/VirtualizerSDK.h>
#include <Windows.h>
#include <lazy_importer.hpp>


namespace hdac::modules
{
    std::vector<uint8_t> MemoryPage::GetData() const
    {
        std::vector<uint8_t> page(end - begin, 0) ;

        std::copy(reinterpret_cast<const uint8_t*>(begin), reinterpret_cast<const uint8_t*>(end), page.begin());

        return page;
    }

    void MemoryScanner::Scan()
    {
        VIRTUALIZER_FALCON_TINY_START
        for (const auto& page : GetExecutableMemoryRegions())
        {
            if (m_checkedPages.contains(page))
                continue;

            //http::HttpPostRequest(xorstr_("http://localhost/detect"), page);

            m_checkedPages.insert(page);
        }
        VIRTUALIZER_FALCON_TINY_END
    }

    std::vector<MemoryPage> MemoryScanner::GetExecutableMemoryRegions()
    {
        VIRTUALIZER_FALCON_TINY_START
        std::vector<MemoryPage> regions;

        const auto IsExecutable = [](const DWORD protection)
        {
            return protection == PAGE_EXECUTE_READ || protection == PAGE_EXECUTE_READWRITE || protection == PAGE_EXECUTE_WRITECOPY;
        };
        // Enumerate memory regions
        MEMORY_BASIC_INFORMATION mbi;
        for (uintptr_t addr = 0; LI_FN(VirtualQuery)(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) == sizeof(mbi); addr += mbi.RegionSize)
        {
            if (!IsExecutable(mbi.Protect))
                continue;

            regions.emplace_back(reinterpret_cast<uintptr_t>(mbi.BaseAddress),
                                 reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize);
        }
        VIRTUALIZER_FALCON_TINY_END
        return regions;
    }
} // hdac
