// c++ includes
#include "pch.h"
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <ctime>

// boost includes
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

using namespace std;

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;
namespace fs = boost::filesystem;
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

const string VERSION = "0.3";

// --------------------------------------------------------------------------
// LOG SYSTEM

void init_logging()
{   
    
    logging::add_file_log(
        keywords::file_name = "JsLoader.log",
        keywords::format = "[%TimeStamp%] %Message%",
        keywords::auto_flush = TRUE
    );

    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

    logging::add_file_log(
        keywords::file_name = "JsLoader.log",
        keywords::format = "[%TimeStamp%] %Message%",
        keywords::auto_flush = TRUE
    );

    logging::add_common_attributes();
}

void makeLog(string severenity, string msg) {
    msg = severenity + " " + msg;

    BOOST_LOG_TRIVIAL(trace) << msg;
}

//------------------------------------------------------------------------------
bool existsFile(string filePath) {
    filePath = ".\\" + filePath;

    return fs::exists(filePath);
}

int getInputMehtod() {
    HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        DWORD threadID = GetWindowThreadProcessId(hwnd, NULL);
        HKL currentLayout = GetKeyboardLayout(threadID);

        unsigned int x = (unsigned int)currentLayout & 0x0000FFFF;

        return ((int)x);
    }
    return 0;
}

void sendCommand(string command) {

    keybd_event(VK_F6, 0, 0, 0);
    keybd_event(VK_F6, 0, KEYEVENTF_KEYUP, 0);

    INPUT ip{};
    ip.type = INPUT_KEYBOARD;
    ip.ki.time = 0;
    ip.ki.dwFlags = KEYEVENTF_UNICODE;
    ip.ki.wVk = 0;
    ip.ki.dwExtraInfo = 0;
    for (size_t i = 0; i < command.size(); ++i)
    {
        ip.ki.wScan = command[i];
        SendInput(1, &ip, sizeof(INPUT));
    }

    keybd_event(VK_RETURN, 0, 0, 0);
    keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);

    return;
}

void writeFile(string filePath, string data) {
    ofstream out(".\\" + filePath);

    if (out.is_open()) {
        out << data;

        makeLog("(info)", "Write file " + filePath + " success...");
    }
    else {
        makeLog("(error)", "Error with write file " + filePath);
    }

    out.close();

    return;
}

string readFile(string filePath) {
    string text, readData;

    boost::filesystem::ifstream fileHandler(".\\" + filePath);

    if (fileHandler.is_open()) {
        while (getline(fileHandler, text)) {
            readData += text;
        }

        makeLog("(info)", "Read file " + filePath + " success...");
    }
    else {
        makeLog("(error)", "Error with reading file " + filePath);
    }

    fileHandler.close();

    return readData;
}

// ---------------------------------------------------------------------------

string getListOfScripts() {
    makeLog("(info)", "Getting list of scripts...");

    string modsPath = ".\\cef\\assets\\mods";
    string listOfScripts = "";
    bool hasFolderFiles = false;

    if (fs::exists(modsPath)) {

        listOfScripts += "[";

        for (auto& p : fs::directory_iterator(modsPath)) {
            string fileExtension = p.path().filename().extension().string();
            string fileName = p.path().filename().string();

            if (fileExtension == ".js") {
                listOfScripts += "\"" + fileName + "\",";
                hasFolderFiles = true;
            }
        }

        listOfScripts += ']';

        makeLog("(info)", "List of scripts was created...");

        if (!hasFolderFiles) {
            makeLog("(info)", "In folder mods no files...");
            return "";
        }

        return listOfScripts.replace(listOfScripts.find(",]"), 2, "]");;
    }
    else {
        makeLog("(info)", "Folder mods is not exist, creating new...");

        fs::create_directory(modsPath);
    }

    return "";
}

void connectJsScripts() {
    makeLog("(info)", "Connecting scripts...");

    string oldHtml, newHtml, text;

    oldHtml = readFile("cef\\assets\\index.html");

    newHtml = oldHtml.substr(0, oldHtml.find("</body>"));

    if (newHtml.find("<!-- js mods -->") != -1) {
        newHtml = oldHtml.erase(oldHtml.find("<!-- js mods -->"));
    }

    if (newHtml.find("<!-- jsLoader-injected -->") == -1) {
        newHtml += "<!-- jsLoader-injected -->";
        newHtml += "<script async src=\"core/jsLoader.js\"></script>";
        newHtml += "</body></html>";

        writeFile("cef\\assets\\index.html", newHtml);
        makeLog("info", "Added jsLoader in HTML file...");
    }

    return;
}

// -------------------------------------------------------------------------------
void initJsLoader()
{
    init_logging();

    makeLog("(info)", "Socket started on port 5469...");

    connectJsScripts();

    try {

        auto const address = net::ip::make_address("127.0.0.1");
        auto const port = static_cast<unsigned short>(std::atoi("5469"));

        net::io_context ioc{ 10 };

        tcp::acceptor acceptor{ ioc, {address, port} };
        for (;;)
        {

            tcp::socket socket{ ioc };

            acceptor.accept(socket);

            std::thread{ std::bind(

                [q{std::move(socket)}]() {



                websocket::stream<tcp::socket> ws{std::move(const_cast<tcp::socket&>(q))};


                ws.set_option(websocket::stream_base::decorator(
                    [](websocket::response_type& res)
                    {
                        res.set(http::field::server,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-server-sync");
                                }));


                ws.accept();

                while (true)
                {
                    try
                    {

                        beast::flat_buffer readBuffer;
                        beast::flat_buffer writeBuffer;


                        ws.read(readBuffer);

                        vector <string> args;
                        auto msg = beast::buffers_to_string(readBuffer.cdata());

                        boost::split(args, msg, [](char c) {return c == '|'; });

                        makeLog("(info)", "Client msg -> " + msg);

                        if (args[0] == "readFile") {
                            makeLog("(info)", "Client requires read file " + args[1]);

                            boost::beast::ostream(writeBuffer) << "readFile|" << readFile(args[1]);
                            ws.write(writeBuffer.data());
                        }

                        if (args[0] == "writeFile") {
                            makeLog("(info)", "Client requires write file " + args[1] + ". With data : " + args[2]);

                            writeFile(args[1], args[2]);
                        }

                        if (args[0] == "sendCommand") {

                            if (args[1][0] == '/') {
                                sendCommand(args[1]);
                            }
                        }

                        if (args[0] == "getVersion") {
                            boost::beast::ostream(writeBuffer) << "version|" << VERSION;
                            ws.write(writeBuffer.data());
                        }

                        if (args[0] == "getKeyboardLayout") {
                            boost::beast::ostream(writeBuffer) << "keyboardLayout|" << getInputMehtod();
                            ws.write(writeBuffer.data());
                        }

                        if (args[0] == "existsFile") {
                            boost::beast::ostream(writeBuffer) << "existsFile|" << existsFile(args[1]);
                            ws.write(writeBuffer.data());
                        }

                        if (args[0] == "getListOfScripts") {
                            boost::beast::ostream(writeBuffer) << "getListOfScripts|" << getListOfScripts();
                            ws.write(writeBuffer.data());
                        }


                        }
                        catch (beast::system_error const)
                        {
                            
                        }
                    }
            }
        ) }.detach();
        }
    }
    catch (beast::system_error const& se) {
        
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
        boost::thread initThread(initJsLoader);
        DisableThreadLibraryCalls(hModule);
        break;
    }
    case DLL_THREAD_ATTACH: 
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}