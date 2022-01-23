#include <pch.h>
#include <iostream>
#include <string>
#include <thread>
#include <filesystem>
#include <fstream>
#include <ctime>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

using namespace std;
namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace attrs = boost::log::attributes;
namespace fs = std::filesystem;
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

const string VERSION = "0.25";
const string LOGFILENAME = "JsLoader.txt";
const string jsScript = "<script>const jsLoader={socket:{isSocketConnected:!1,socket:null,connectToSocket(){this.socket=new WebSocket(\"ws://localhost:5469\"),this.socket.onopen=function(){jsLoader.socket.sendEvent(\"getVersion\",t=>{jsLoader.showAddedScript(`JsLoader ver.${t.data.split(\"|\")[1]}`,\"success\")}),jsLoader.log.makeLog(\"JsLoader\",`(info) Socket connected...`),jsLoader.socket.isSocketConnected=!0},this.socket.onerror=function(t){jsLoader.showAddedScript(\"JsLoader\",\"warning\"),jsLoader.log.makeLog(\"JsLoader\",`(error) With socket...`)}},sendEvent(t,e){this.socket.onmessage=e,this.socket.send(t)}},log:{scriptLogs:[],start(){document.addEventListener(\"keyup\",t=>{t.ctrlKey&&t.shiftKey&&76==t.keyCode&&this.showDialog()})},makeLog(t,e){console.log(t,'->>', e); e = `${jsLoader.getTimeStamp()} ${e} <n>`;let s=this.scriptLogs.filter(e=>t==e.scriptName);0!=s.length?1!=s.length||(s[0].logs+=e):this.scriptLogs.push({scriptName:t,logs:e})},hookDialogButtons(){try{App.$children[0].$children[1].$children[0].open=new Proxy(App.$children[0].$children[1].$children[0].open,{apply:(t,e,s)=>(jsLoader.log.showLog(App.$children[0].$children[1].$children[0].items[s[0]][0]),App.$children[0].$children[1].$children[0].back(),0)}),App.$children[0].$children[1].$children[0].back=new Proxy(App.$children[0].$children[1].$children[0].back,{apply(t,e,s){Reflect.apply(t,e,s)}})}catch(t){}},showDialog(){let t=\"\";this.scriptLogs.forEach(e=>{t+=`${e.scriptName}<n>`});try{addDialogInQueue('[0,2,\"Logs\",\"\",\"Select\",\"Close\",0,0]',t,0),setTimeout(()=>{this.hookDialogButtons()},150)}catch(t){}},showLog(t){let e=this.scriptLogs.filter(e=>t==e.scriptName);console.log(e[0]),setTimeout(()=>addDialogInQueue(`[0,0,\"${e[0].scriptName}\",\"\",\"\",\"Close\",0,0]`,e[0].logs,0),100)}},icons:{info:{url:'<svg class=\"script__icon-img\" xmlns=\"http://www.w3.org/2000/svg\" x=\"0px\" y=\"0px\" width=\"144\" height=\"144\" viewBox=\"0 0 48 48\" style=\" fill:#000000;\"><linearGradient id=\"Z3eIuf5QY2EetuA~FfDd6a_VQOfeAx5KWTK_gr1\" x1=\"9.899\" x2=\"38.183\" y1=\"9.98\" y2=\"38.264\" gradientUnits=\"userSpaceOnUse\"><stop offset=\"0\" stop-color=\"#33bef0\"></stop><stop offset=\"1\" stop-color=\"#0a85d9\"></stop></linearGradient><path fill=\"url(#Z3eIuf5QY2EetuA~FfDd6a_VQOfeAx5KWTK_gr1)\" d=\"M44.041,24.122c0,11.045-8.955,20-20,20s-20-8.955-20-20s8.955-20,20-20\tS44.041,13.077,44.041,24.122z\"></path><path d=\"M22,36h4c0.552,0,1-0.448,1-1V20c0-0.552-0.448-1-1-1h-4c-0.552,0-1,0.448-1,1v15\tC21,35.552,21.448,36,22,36z\" opacity=\".05\"></path><path d=\"M22.227,35.5h3.547c0.401,0,0.727-0.325,0.727-0.727V20.227c0-0.401-0.325-0.727-0.727-0.727h-3.547\tc-0.401,0-0.727,0.325-0.727,0.727v14.547C21.5,35.175,21.825,35.5,22.227,35.5z\" opacity=\".07\"></path><radialGradient id=\"Z3eIuf5QY2EetuA~FfDd6b\" cx=\"24\" cy=\"16\" r=\"5.108\" gradientTransform=\"matrix(.7808 0 0 .7066 5.26 4.096)\" gradientUnits=\"userSpaceOnUse\"><stop offset=\".516\"></stop><stop offset=\"1\" stop-opacity=\"0\"></stop></radialGradient><ellipse cx=\"24\" cy=\"15.402\" fill=\"url(#undefined)\" opacity=\".15\" rx=\"3.988\" ry=\"3.609\"></ellipse><path fill=\"#fff\" d=\"M24,17.732c1.7,0,2.65-1.068,2.65-2.388C26.65,14.024,25.647,13,24,13s-2.65,1.024-2.65,2.344\tC21.35,16.664,22.3,17.732,24,17.732z\"></path><rect width=\"4\" height=\"15\" x=\"22\" y=\"20\" fill=\"#fff\"></rect></svg>',color:\"hsl(201deg 76% 50%)\"},warning:{url:'<svg class=\"script__icon-img\" xmlns=\"http://www.w3.org/2000/svg\" x=\"0px\" y=\"0px\" width=\"144\" height=\"144\" viewBox=\"0 0 48 48\" style=\" fill:#000000;\"><linearGradient id=\"wRKXFJsqHCxLE9yyOYHkza_fYgQxDaH069W_gr1\" x1=\"9.858\" x2=\"38.142\" y1=\"9.858\" y2=\"38.142\" gradientUnits=\"userSpaceOnUse\"><stop offset=\"0\" stop-color=\"#f44f5a\"></stop><stop offset=\".443\" stop-color=\"#ee3d4a\"></stop><stop offset=\"1\" stop-color=\"#e52030\"></stop></linearGradient><path fill=\"url(#wRKXFJsqHCxLE9yyOYHkza_fYgQxDaH069W_gr1)\" d=\"M44,24c0,11.045-8.955,20-20,20S4,35.045,4,24S12.955,4,24,4S44,12.955,44,24z\"></path><path d=\"M33.192,28.95L28.243,24l4.95-4.95c0.781-0.781,0.781-2.047,0-2.828l-1.414-1.414\tc-0.781-0.781-2.047-0.781-2.828,0L24,19.757l-4.95-4.95c-0.781-0.781-2.047-0.781-2.828,0l-1.414,1.414\tc-0.781,0.781-0.781,2.047,0,2.828l4.95,4.95l-4.95,4.95c-0.781,0.781-0.781,2.047,0,2.828l1.414,1.414\tc0.781,0.781,2.047,0.781,2.828,0l4.95-4.95l4.95,4.95c0.781,0.781,2.047,0.781,2.828,0l1.414-1.414\tC33.973,30.997,33.973,29.731,33.192,28.95z\" opacity=\".05\"></path><path d=\"M32.839,29.303L27.536,24l5.303-5.303c0.586-0.586,0.586-1.536,0-2.121l-1.414-1.414\tc-0.586-0.586-1.536-0.586-2.121,0L24,20.464l-5.303-5.303c-0.586-0.586-1.536-0.586-2.121,0l-1.414,1.414\tc-0.586,0.586-0.586,1.536,0,2.121L20.464,24l-5.303,5.303c-0.586,0.586-0.586,1.536,0,2.121l1.414,1.414\tc0.586,0.586,1.536,0.586,2.121,0L24,27.536l5.303,5.303c0.586,0.586,1.536,0.586,2.121,0l1.414-1.414\tC33.425,30.839,33.425,29.889,32.839,29.303z\" opacity=\".07\"></path><path fill=\"#fff\" d=\"M31.071,15.515l1.414,1.414c0.391,0.391,0.391,1.024,0,1.414L18.343,32.485\tc-0.391,0.391-1.024,0.391-1.414,0l-1.414-1.414c-0.391-0.391-0.391-1.024,0-1.414l14.142-14.142\tC30.047,15.124,30.681,15.124,31.071,15.515z\"></path><path fill=\"#fff\" d=\"M32.485,31.071l-1.414,1.414c-0.391,0.391-1.024,0.391-1.414,0L15.515,18.343\tc-0.391-0.391-0.391-1.024,0-1.414l1.414-1.414c0.391-0.391,1.024-0.391,1.414,0l14.142,14.142\tC32.876,30.047,32.876,30.681,32.485,31.071z\"></path></svg>',color:\"hsl(355deg 81% 55%)\"},success:{url:'<svg class=\"script__icon-img\" xmlns=\"http://www.w3.org/2000/svg\" x=\"0px\" y=\"0px\" width=\"144\" height=\"144\" viewBox=\"0 0 48 48\" style=\" fill:#000000;\"><linearGradient id=\"I9GV0SozQFknxHSR6DCx5a_70yRC8npwT3d_gr1\" x1=\"9.858\" x2=\"38.142\" y1=\"9.858\" y2=\"38.142\" gradientUnits=\"userSpaceOnUse\"><stop offset=\"0\" stop-color=\"#21ad64\"></stop><stop offset=\"1\" stop-color=\"#088242\"></stop></linearGradient><path fill=\"url(#I9GV0SozQFknxHSR6DCx5a_70yRC8npwT3d_gr1)\" d=\"M44,24c0,11.045-8.955,20-20,20S4,35.045,4,24S12.955,4,24,4S44,12.955,44,24z\"></path><path d=\"M32.172,16.172L22,26.344l-5.172-5.172c-0.781-0.781-2.047-0.781-2.828,0l-1.414,1.414\tc-0.781,0.781-0.781,2.047,0,2.828l8,8c0.781,0.781,2.047,0.781,2.828,0l13-13c0.781-0.781,0.781-2.047,0-2.828L35,16.172\tC34.219,15.391,32.953,15.391,32.172,16.172z\" opacity=\".05\"></path><path d=\"M20.939,33.061l-8-8c-0.586-0.586-0.586-1.536,0-2.121l1.414-1.414c0.586-0.586,1.536-0.586,2.121,0\tL22,27.051l10.525-10.525c0.586-0.586,1.536-0.586,2.121,0l1.414,1.414c0.586,0.586,0.586,1.536,0,2.121l-13,13\tC22.475,33.646,21.525,33.646,20.939,33.061z\" opacity=\".07\"></path><path fill=\"#fff\" d=\"M21.293,32.707l-8-8c-0.391-0.391-0.391-1.024,0-1.414l1.414-1.414c0.391-0.391,1.024-0.391,1.414,0\tL22,27.758l10.879-10.879c0.391-0.391,1.024-0.391,1.414,0l1.414,1.414c0.391,0.391,0.391,1.024,0,1.414l-13,13\tC22.317,33.098,21.683,33.098,21.293,32.707z\"></path></svg>',color:\"hsl(149deg 37% 47%)\"}},wrapperElem:null,sleep:t=>new Promise(e=>setTimeout(e,t)),setStyles(){const t=document.createElement(\"style\");t.innerHTML=\"@import url('https://fonts.googleapis.com/css2?family=Montserrat:wght@300;600&display=swap');#addedScripts{     position: absolute;     right: 0;     bottom: 0;     width: 50%;     height: 100%;     display: flex;     flex-direction: column;     align-items: flex-end;     justify-content: flex-end;}.js-script{     font-family: 'Montserrat', sans-serif;     background: hsl(0deg 0% 100%);     overflow: hidden;     display: flex;     align-items: center;     width: 27.5vh;     height: 4.8vh;     box-shadow: rgba(0, 0, 0, 0.25) 0px 25px 50px -12px;     border-radius: 0.7vh;     padding: 1.1vh;     margin-bottom: 4vh;     transition: transform 1.05s cubic-bezier(0.6, -0.28, 0.3, 0.96);}.js-script::after{     content: '';     position: absolute;     right: 0;     bottom: -0.75vh;     width: 0%;     height: 1vh;     background: hsl(199deg 100% 34%);     transition: 3s cubic-bezier(0.44, 0.43, 0.01, 0.01);     border-radius: 3vh;}.js-script.js-startTimer::after{     width: 100%;}.js-script.js-showed{     transform: translateX(-4.5vh);}.js-script.js-hidden{     transform: translateX(70vh);}.script__indicator{     width: 0.4vh;     height: 100%;     border-radius: 4vh;     box-shadow: inset 0 0 10px 0 #6c6c6c2e;}.script__data{     display: flex;     flex-direction: column;     align-items: flex-start;}.script__name{     font-size: 1.6vh;     line-height: 2.3vh;     font-weight: 550;}.script__status{     font-size: 1.4vh;     color: hsl(0deg 0% 51%);     font-weight: 455;}.script__icon{     display: flex;     width: 3.3vh;     height: 3.3vh;     margin: 0 1.7vh 0 1.5vh;}.script__icon-img{     width: 100%;     height: 100%;}          \",document.head.append(t)},createWrapper(){try{this.wrapperElem=document.createElement(\"div\"),this.wrapperElem.id=\"addedScripts\",document.getElementById(\"app\").append(this.wrapperElem),jsLoader.log.makeLog(\"JsLoader\",`(info) Wrapper created...`)}catch(t){jsLoader.log.makeLog(\"JsLoader\",`(error) Wrapper: ${t.message}`)}},async showAddedScript(t,e){try{let s=\"Loaded\";const o=document.createElement(\"div\");\"warning\"===e&&(s=\"Not loaded\"),o.className=`js-script js-${e} js-hidden`,o.innerHTML=`<div class=\"script__indicator\" style=\"background-color:${this.icons[e].color}\"></div><div class=\"script__icon\">     ${this.icons[e].url}</div><div class=\"script__data\">     <div class=\"script__name\">${t}</div>     <div class=\"script__status\">${s}</div><div>`,this.sleep(10).then(()=>{o.classList.remove(\"js-hidden\"),o.classList.add(\"js-showed\")}).then(()=>this.sleep(800)).then(()=>o.classList.add(\"js-startTimer\")).then(()=>this.sleep(3300)).then(()=>{o.classList.remove(\"js-showed\"),o.classList.add(\"js-hidden\")}).then(()=>this.sleep(5e3)).then(()=>o.remove()),this.wrapperElem.append(o),jsLoader.log.makeLog(\"JsLoader\",`(info) ShowAddedScript ${t}...`)}catch(t){jsLoader.log.makeLog(\"JsLoader\",`(error) When showAdded script ${t.message}`)}},getTimeStamp(){const t=new Date;return`[${t.getHours()}:${t.getMinutes()}:${t.getSeconds()}.${t.getMilliseconds()}]`},start(){try{setTimeout(()=>{this.socket.connectToSocket(),this.createWrapper(),this.setStyles(),this.log.start(),jsLoader.log.makeLog(\"JsLoader\",`(info) Started...`)},600)}catch(t){this.log.makeLog(\"JsLoader\",`(error) Restarting...`),this.start()}}};document.addEventListener(\"DOMContentLoaded\",jsLoader.start()),document.removeEventListener(\"DOMContentLoaded\",jsLoader.start);</script>";

// --------------------------------------------------------------------------
// LOG SYSTEM

void init_logging()
{
    cout << "LOG SYSTEM INITED!";

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
    // нажатие F6
    keybd_event(VK_F6, 0, 0, 0);
    keybd_event(VK_F6, 0, KEYEVENTF_KEYUP, 0);

    // набор команды в чат
    INPUT ip;
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

    // нажатие Enter - отправка команды
    keybd_event(VK_RETURN, 0, 0, 0);
    keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);

    return;
}

void writeFile(string filePath, string data) {
    ofstream out(".\\" + filePath);

    if (out.is_open()) {
        out << data;

        if (filePath != LOGFILENAME) makeLog("(info)", "Write file " + filePath + " success...");
    }
    else {
        if (filePath != LOGFILENAME) makeLog("(error)", "Error with write file " + filePath);
    }

    out.close();

    return;
}

string readFile(string filePath) {
    string text, readData;

    ifstream in(".\\" + filePath);

    if (in.is_open()) {
        while (getline(in, text)) {
            readData += text;
        }

        if (filePath != LOGFILENAME) makeLog("(info)", "Read file " + filePath + " success...");
    }
    else {
        if (filePath != LOGFILENAME) makeLog("(error)", "Error with reading file " + filePath);
    }

    in.close();

    return readData;
}

// ---------------------------------------------------------------------------

vector <string> getListOfScripts() {
    makeLog("(info)", "Getting list of scripts...");

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

        makeLog("(info)", "List of scripts was created...");

        return listOfScripts;
    }
    else {
        makeLog("(info)", "Folder mods is not exist, creating new...");

        fs::create_directory(modsPath);
    }

    return listOfScripts;
}

void connectJsScripts() {
    makeLog("(info)", "Connecting scripts...");

    string oldHtml, newHtml, text;
    vector <string> listOfScripts;

    listOfScripts = getListOfScripts();

    oldHtml = readFile("cef\\assets\\index.html");

    newHtml = oldHtml.substr(0, oldHtml.find("</body>"));

    if (listOfScripts.size() == 0) {
        makeLog("(info)", "No scripts in folder...");

        if (newHtml.find("<script>const") == -1) {
            newHtml += jsScript;
            newHtml += "</body></html>";

            writeFile("cef\\assets\\index.html", newHtml);
        }
        return;
    }

    if (oldHtml.find("<!-- js mods -->") != -1) newHtml = oldHtml.substr(0, oldHtml.find("<!-- js mods -->"));

    newHtml += "<!-- js mods -->";

    for (auto script : listOfScripts) {
        newHtml += "<script src=\"mods/" + script + "\"></script>";

        makeLog("(info)", "Added new script -> " + script);
    }

    if (newHtml.find("<script>const") == -1) newHtml += jsScript;

    newHtml += "</body></html>";
    
    writeFile("cef\\assets\\index.html", newHtml);

    return;
}

// -------------------------------------------------------------------------------
void startSocket()
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
    catch (beast::system_error const& se) {
        cout << "ERROR: " + *se.what() << endl;
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
        std::thread(startSocket).detach();
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}