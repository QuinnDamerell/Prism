#pragma once

#include "Common.h"

#include "mongoose.h"
#include "Timer.h"
#include "SharedFromThisHelper.h"
#include "IWebCommandHandler.h"

DECLARE_SMARTPOINTER(WebServer);
class WebServer :
    public LightFx::ITimerCallback,
    public LightFx::SharedFromThis,
    public IWebCommandHandler
{
public:
    WebServer() :
        m_http_server_opts{0},
        m_eventManager{0},
        m_connection(nullptr),
        m_isInited(false)
    {};

    ~WebServer();

    // Sets up the web server
    void Setup();

    // Starts it
    void Start();

    // Fired when a web call has been made
    void HandleWebCall(struct mg_connection *nc, int ev, void *ev_data) override;

private:
    // The port we will try to bind.
    const char* m_http_port = "80";
    const char* m_http_port_backup = "3008";

    // Our current server options
    struct mg_serve_http_opts m_http_server_opts;   

    // Event Manager
    struct mg_mgr m_eventManager;

    // A pointer to our connection
    struct mg_connection* m_connection;

    // Indicates if we are inited or not
    bool m_isInited;

    // Our poll thread
    LightFx::TimerPtr m_pollTimer;

    // Fired when our poll timer comes back
    void OnTimerTick(std::chrono::milliseconds elapsedTime);

    // Handles a web command
    void WebServer::HandleWebCommand(const char* jsonStr, int length);
};