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
    // Check intensity
    if (CheckConfigChange(oldConfig, newConfig, "Intensity") == ConfigChangeType::Updated)
    {
        if (auto handler = m_commandHandler.lock())
        {
            handler->IntensityChanged(newConfig["Intensity"].GetDouble());
        }
    }

    // Check for enabled gem updates
    if (CheckConfigChange(oldConfig, newConfig, "EnabledGems") == ConfigChangeType::Updated)
    {
        if (auto handler = m_commandHandler.lock())
        {
            handler->EnabledGemsChanged();
        }
    }   

    // Check for running time updates
    if (CheckConfigChange(oldConfig, newConfig, "MaxGemRunningTimeSeconds") == ConfigChangeType::Updated ||
        CheckConfigChange(oldConfig, newConfig, "MinGemRunningTimeSeconds") == ConfigChangeType::Updated)
    {
        if (auto handler = m_commandHandler.lock())
        {
            handler->GemRunningTimeChanged();
        }
    }
}

// Fired when a command has been issued
void RapcomHost::OnCommand(rapidjson::Document& request, rapidjson::Document& response)
{

}
