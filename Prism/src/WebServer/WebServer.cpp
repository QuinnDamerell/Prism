#include <string>
#include <iostream>
#include <map>

#include "WebServer/WebServer.h"

using namespace LightFx;

// Used to reference web servers.
static IWebCommandHandlerWeakPtr s_webServer;

void static event_hander(struct mg_connection *nc, int ev, void *ev_data)
{
    IWebCommandHandlerPtr commandHandler = s_webServer.lock();
    if (commandHandler)
    {
        commandHandler->HandleWebCall(nc, ev, ev_data);
    }    
}
    
WebServer::~WebServer()
{
    // Kill the poll thread.
    if (m_pollTimer)
    {
        m_pollTimer->Stop();
        m_pollTimer = nullptr;
    }

    // Uninit
    if (m_isInited)
    {
        mg_mgr_free(&m_eventManager);
    }
}

void WebServer::Setup()
{
    // Init the event manager
    mg_mgr_init(&m_eventManager, NULL);
    m_isInited = true;

    static const char *root = "c:/";
    m_http_server_opts.document_root = root;

    // Bind
    m_connection = mg_bind(&m_eventManager, m_http_port, event_hander);
    if (m_connection == nullptr)
    {
        std::cout << "Failed to bind port 80, trying 3001\r\n";
        m_connection = mg_bind(&m_eventManager, m_http_port_backup, event_hander);
        if (m_connection == nullptr)
        {
            std::cout << "Failed to bind port 3001, killing the webserver\r\n";
            m_connection = nullptr;
            mg_mgr_free(&m_eventManager);
            m_isInited = false;
            return;
        }
        else
        {
            std::cout << "Web server started on port 300\r\n";
        }
    }
    else
    {
        std::cout << "Web server started on port 80\r\n";

    }

    // Finish the setup
    mg_set_protocol_http_websocket(m_connection);

    // Set ourself as the static web server.
    s_webServer = GetWeakPtr<IWebCommandHandler>();
}

void WebServer::Start()
{
    if (!m_pollTimer)
    {
        // Make a new timer
        m_pollTimer = std::make_shared<Timer>(GetSharedPtr<ITimerCallback>(), milliseconds(0));

        // Start it
        m_pollTimer->Start();
    }
}

void WebServer::OnTimerTick(milliseconds elapsedTime)
{
    if (m_isInited)
    {
        // Poll when the timer fires.
        mg_mgr_poll(&m_eventManager, 10000);
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void WebServer::HandleWebCall(struct mg_connection *nc, int ev, void *ev_data)
{
    struct http_message *hm = (struct http_message *) ev_data;

    switch (ev) {
    case MG_EV_HTTP_REQUEST:
        // Check for an incoming command
        if (mg_vcmp(&hm->uri, "/api/v1/command") == 0) 
        {
            HandleWebCommand(hm->body.p, hm->body.len);
        }
        else 
        {
            /* Serve static content */
            mg_serve_http(nc, hm, m_http_server_opts); 
        }
        break;
    default:
        break;
    }
}

void WebServer::HandleWebCommand(const char* jsonStr, int length)
{

}
