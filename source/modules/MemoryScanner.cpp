//
// Created by Orange on 12/9/2024.
//

#include "MemoryScanner.hpp"

#include <future>
#include <iomanip>
#include <CodeVirtualizer/VirtualizerSDK.h>
#include <Windows.h>
#include <lazy_importer.hpp>
#include <print>
#include <sstream>
#include <openssl/evp.h>
#include <boost/crc.hpp>
#include <boost/asio/post.hpp>
#include <api/ApiService.hpp>
#include <zlib.h>
#include <boost/beast/zlib/error.hpp>


std::vector<uint8_t> compressData(const std::vector<uint8_t>& data)
{
    // Estimate the maximum size for compressed data
    const auto uncompressedData = (size_t) compressBound((uLong) data.size());
    std::vector<uint8_t> compressedData(uncompressedData);

    // Compress the data
    uLongf comp = (uLongf) compressedData.size();
    int result = compress(compressedData.data(), &comp, data.data(), (uLong) data.size());
    if (result != Z_OK)
    {
        throw std::runtime_error("Compression failed: " + std::to_string(result));
    }

    // Resize the compressedData vector to the actual size
    compressedData.resize(comp);
    return compressedData;
}


std::vector<uint8_t> MemoryPage::GetData() const
{
    std::vector<uint8_t> page(end - begin, 0);

    std::copy(reinterpret_cast<const uint8_t*>(begin), reinterpret_cast<const uint8_t*>(end), page.begin());

    return page;
}

size_t MemoryPage::Size() const
{
    return end - begin;
}

uint32_t MemoryPage::Crc32() const
{
    boost::crc_32_type result;
    result.process_bytes(reinterpret_cast<const uint8_t*>(begin), Size());
    return result.checksum();
}


namespace hdac::modules
{
    MemoryScanner::MemoryScanner(boost::asio::io_context& io_service) : m_ioContext(io_service)
    {
    }

    void MemoryScanner::Scan()
    {
        VIRTUALIZER_FALCON_TINY_START
        const auto pages = GetExecutableMemoryRegions();

        std::promise<void> promise;
        const std::future<void> future = promise.get_future();
        std::atomic pagesLeft = pages.size();


        for (const auto& page: pages)
        {
            boost::asio::post(m_ioContext, [this, &page, &pagesLeft, &promise]()
            {
                VIRTUALIZER_FALCON_TINY_START
                const auto pageHash = page.Crc32(); {
                    std::shared_lock lock(m_mutex);
                    if (m_checkedPages.contains(pageHash))
                    {
                        if (--pagesLeft == 0)
                            promise.set_value();
                        return;
                    }
                }

                //http::HttpPostRequest(xorstr_("http://localhost/detect"), page);

                static api::ApiService apiService;

                if (const auto detect = apiService.SendPageToAnalysis(compressData(page.GetData())))

                    MessageBox(nullptr, std::format(R"(AC system detected cheat "{}" using {}")", detect->cheatName, detect->method).c_str(), "Hammer Down", MB_OK | MB_ICONERROR);
                {
                    std::unique_lock lock(m_mutex);
                    m_checkedPages.insert(pageHash);
                }
                if (--pagesLeft == 0)
                    promise.set_value();
                VIRTUALIZER_FALCON_TINY_END
            });
        }
        future.wait();
        VIRTUALIZER_FALCON_TINY_END
    }

    std::vector<MemoryPage> MemoryScanner::GetExecutableMemoryRegions()
    {
        std::vector<MemoryPage> regions;

        const auto IsExecutable = [](const DWORD protection)
        {
            return protection == PAGE_EXECUTE_READ || protection == PAGE_EXECUTE_READWRITE || protection ==
                   PAGE_EXECUTE_WRITECOPY || protection == PAGE_EXECUTE;
        };
        // Enumerate memory regions
        MEMORY_BASIC_INFORMATION mbi;
        for (uintptr_t addr = 0; LI_FN(VirtualQuery)(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) == sizeof(mbi);
             addr += mbi.RegionSize)
        {
            if (!IsExecutable(mbi.Protect))
                continue;

            regions.emplace_back(reinterpret_cast<uintptr_t>(mbi.BaseAddress),
                                 reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize);
        }
        return regions;
    }
} // hdac
