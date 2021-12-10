﻿// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include <iostream>
#include <string>
#include <thread>
#include <filesystem>
#include <fstream>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace fs = std::filesystem;
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------
void writeFile(string filePath, string data) {
    ofstream out(filePath);

    if (out.is_open()) {
        out << data;

        BOOST_LOG_TRIVIAL(info) << "Write success " + filePath;
    }
    else {
        BOOST_LOG_TRIVIAL(error) << "Error with writing file " + filePath;
    }

    out.close();

    return;
}

string readFile(string filePath) {
    string text, readData;

    ifstream in(filePath);

    if (in.is_open()) {
        while (getline(in, text)) {
            readData += text;
        }

        BOOST_LOG_TRIVIAL(info) << "Read success " + filePath;
    }
    else {
        BOOST_LOG_TRIVIAL(error) << "Error with reading file " + filePath;
    }

    in.close();

    return readData;
}

void init_logging()
{
    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

    logging::add_file_log
    (
        keywords::file_name = "JSLoader.log",
        keywords::format = "[%TimeStamp%] [%Severity%] %Message%"
    );

    logging::add_common_attributes();
}

vector <string> getListOfScripts() {
    BOOST_LOG_TRIVIAL(info) << "Getting list of scripts in folder mods...";

    string modsPath = ".\\cef\\assets\\mods";
    vector <string> listOfScripts;

    if (fs::exists(modsPath)) {

        for (auto p : fs::directory_iterator(modsPath)) {
            string fileExtension = p.path().filename().extension().u8string();
            string fileName = p.path().filename().u8string();

            if (fileExtension == ".js") {
                listOfScripts.push_back(fileName);
            }
        }

        BOOST_LOG_TRIVIAL(info) << "List of script was created...";

        return listOfScripts;
    }
    else {
        BOOST_LOG_TRIVIAL(info) << "No folder mods, creating...";

        fs::create_directory(".\\cef\\assets\\mods");
    }

    return listOfScripts;
}

void connectJsScripts() {
    BOOST_LOG_TRIVIAL(info) << "Started connect js scripts...";

    string oldHtml, newHtml, text;
    bool needReplaceHtml = false;

    oldHtml = readFile(".\\cef\\assets\\index.html");

    newHtml = oldHtml.substr(0, oldHtml.find("</body>"));

    for (auto script : getListOfScripts()) {

        if (oldHtml.find(script) == -1) {
            needReplaceHtml = true;

            newHtml += "<script src=\"mods\\" + script + "\"></script>";

            BOOST_LOG_TRIVIAL(info) << "Added script -> " + script;
        }

    }

    newHtml += "</body></html>";

    if (needReplaceHtml) {
        writeFile(".\\cef\\assets\\index.html", newHtml);
    }

    cout << newHtml << needReplaceHtml;
}

void startSocket(void* pvParams)
{

    connectJsScripts();
    init_logging();

    BOOST_LOG_TRIVIAL(info) << "Socket has started on port 2011...";

    auto const address = net::ip::make_address("127.0.0.1");
    auto const port = static_cast<unsigned short>(std::atoi("2011"));

    net::io_context ioc{ 10 };

    tcp::acceptor acceptor{ ioc, {address, port} };
    for (;;)
    {

        tcp::socket socket{ ioc };

        acceptor.accept(socket);

        std::thread{ std::bind(
            //[q = std::move(socket)]() mutable { // socket will be const - mutable should be used
            [q{std::move(socket)}]() { // socket will be const - mutable should be used



            websocket::stream<tcp::socket> ws{std::move(const_cast<tcp::socket&>(q))};

            // Set a decorator to change the Server of the handshake
            // no need to set. It ıs not necessary
            ws.set_option(websocket::stream_base::decorator(
                [](websocket::response_type& res)
                {
                    res.set(http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-server-sync");
                            }));

            // Accept the websocket handshake
            ws.accept();

            while (true)
            {
                try
                {

                    beast::flat_buffer readBuffer;
                    beast::flat_buffer writeBuffer;

                    // Read a message
                    ws.read(readBuffer);

                    vector <string> args;
                    auto msg = beast::buffers_to_string(readBuffer.cdata());

                    boost::split(args, msg, [](char c) {return c == '|'; });

                    BOOST_LOG_TRIVIAL(info) << "Client msg -> " + msg;

                    if (args[0] == "readFile") {
                        BOOST_LOG_TRIVIAL(info) << "Client requires read file " + args[1];

                        boost::beast::ostream(writeBuffer) << readFile(args[1]);
                        ws.write(writeBuffer.data());
                    }

                    if (args[0] == "writeFile") {
                        BOOST_LOG_TRIVIAL(info) << "Client requires write file " + args[1] + ". With data : " + args[2];

                        writeFile(args[1], args[2]);
                    }

                    if (args[0] == "userConnected") {
                        BOOST_LOG_TRIVIAL(info) << "User connected...";

                        boost::beast::ostream(writeBuffer) << "connection established";
                        ws.write(writeBuffer.data());
                    }

                    // Echo the message back
                    //ws.text(ws.got_text());
                    //bost::beast::ostream(buffer) << "something";

                    }
                    catch (beast::system_error const& se)
                    {
                        if (se.code() != websocket::error::closed)
                        {
                            std::cerr << "Error: " << se.code().message() << std::endl;
                            break;
                        }
                    }
                }
        }
    ) }.detach();
    }
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        _beginthread(startSocket, NULL, NULL);
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
