#include "RapcomHost.h"

using namespace Rapcom;

RapcomHost::RapcomHost(IControlCommandHandlerWeakPtr commandHandler) :
    m_commandHandler(commandHandler)
{ };

void RapcomHost::Initialize()
{
    // Make the rapcom object
    m_rapcom = std::make_shared<RapcomBase>("Prism", GetWeakPtr<IRapcomListener>());

    // Start up the servers
    m_rapcom->Start();
}

// Fired when the config has been changed.
void RapcomHost::OnConfigChange(rapidjson::Document& oldConfig, rapidjson::Document& newConfig)
{

}

// Fired when a command has been issued
void RapcomHost::OnCommand(rapidjson::Document& request, rapidjson::Document& response)
{

}
