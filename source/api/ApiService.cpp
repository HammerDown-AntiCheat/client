//
// Created by Orange on 12/10/2024.
//
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include "ApiService.hpp"
#include <string>
#include <xorstr.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <nlohmann/json.hpp>
#include <CodeVirtualizer/VirtualizerSDK.h>


namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = net::ip::tcp; // from <boost/asio/ip/tcp.hpp

std::optional<hdac::api::DetectInfo> hdac::api::ApiService::SendPageToAnalysis(const std::vector<uint8_t>& page) const
{

    VIRTUALIZER_FALCON_TINY_START
    try
    {
            std::scoped_lock lock(m_mutex);
            // Parsing the URL
            std::string host, target;
            unsigned short port = 80;


            // Simple URL parsing for HTTP, for HTTPS modify accordingly
            if (boost::starts_with(url, xorstr_("http://")))
            {
                target = url.substr(7);
            } else if (boost::starts_with(url, xorstr_("https://")))
            {
                target = url.substr(8);
                port = 443;
            } else
                return {};

            // Extract host and path (target)
            size_t host_end = target.find('/');
            if (host_end == std::string::npos)
            {
                host = target;
                target = "/";
            } else
            {
                host = target.substr(0, host_end);
                target = target.substr(host_end);
            }

            // Setup Boost.Asio IO context
            net::io_context ioc;

            // These objects perform the resolution and connection
            tcp::resolver resolver(ioc);
            beast::tcp_stream stream(ioc);

            // Look up the domain name
            auto const results = resolver.resolve(host, std::to_string(port));


            stream.connect(results);

            // Set up an HTTP GET request message
            beast::http::request<beast::http::vector_body<uint8_t>> req{beast::http::verb::post, target, 11};
            req.set(beast::http::field::host, host);
            req.set(beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            req.set(beast::http::field::content_type, "text/plain");

            req.body() = page;
            req.prepare_payload();
            // Send the HTTP request to the remote host
            beast::http::write(stream, req);

            // This buffer is used for reading and must be persisted
            beast::flat_buffer buffer;

            // Declare a container to hold the response
            beast::http::response<beast::http::string_body> res;

            // Receive the HTTP response
            beast::http::read(stream, buffer, res);

            // Close the connection gracefully
            beast::error_code ec;
            stream.socket().shutdown(tcp::socket::shutdown_both, ec);

            const auto json = nlohmann::json::parse(res.body());

            if (!json.at("detected").get<bool>())
                return std::nullopt;

            return DetectInfo{json.at("cheat_name"), json.at("method")};
        }
        catch (std::exception const& e)
        {
            MessageBoxA(nullptr, e.what(), "Error", MB_OK);
        }

    VIRTUALIZER_FALCON_TINY_END;
    return std::nullopt;
}
