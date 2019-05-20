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

    // Check for active hours updates
    if (CheckConfigChange(oldConfig, newConfig, "ActiveHoursOffHour") == ConfigChangeType::Updated ||
        CheckConfigChange(oldConfig, newConfig, "ActiveHoursOffMin") == ConfigChangeType::Updated ||
        CheckConfigChange(oldConfig, newConfig, "ActiveHoursOffTimeOfDay") == ConfigChangeType::Updated || 
        CheckConfigChange(oldConfig, newConfig, "ActiveHoursOnHour") == ConfigChangeType::Updated || 
        CheckConfigChange(oldConfig, newConfig, "ActiveHoursOnMin") == ConfigChangeType::Updated || 
        CheckConfigChange(oldConfig, newConfig, "ActiveHoursOnTimeOfDay") == ConfigChangeType::Updated)
    {
        if (auto handler = m_commandHandler.lock())
        {
            handler->ActiveHoursUpdate();
        }
    }
}

// Fired when a command has been issued
void RapcomHost::OnCommand(rapidjson::Document& request, rapidjson::Document& response)
{
	// Look for the command name
	auto commandValue = request.FindMember("Command");
	if (commandValue == request.MemberEnd())
	{
		return;
	}
	if (!commandValue->value.IsString())
	{
		return;
	}
	std::string command(commandValue->value.GetString());

	// Look for a realtime control
	if (command.compare("RealTimeControl") == 0)
	{
		// There should be values sent.
		float value1 = -1;
		float value2 = -1;
		float value3 = -1;
		float value4 = -1;
		auto value = request.FindMember("Value1");
		if (value != request.MemberEnd())
		{
			auto type = value->value.GetType();
			if (value->value.IsFloat())
			{
				value1 = value->value.GetFloat();
			}
		}
		value = request.FindMember("Value2");
		if (value != request.MemberEnd())
		{
			if (value->value.IsFloat())
			{
				value2 = value->value.GetFloat();
			}
		}
		value = request.FindMember("Value3");
		if (value != request.MemberEnd())
		{
			if (value->value.IsFloat())
			{
				value3 = value->value.GetFloat();
			}
		}
		value = request.FindMember("Value4");
		if (value != request.MemberEnd())
		{
			if (value->value.IsFloat())
			{
				value4 = value->value.GetFloat();
			}
		}
		if (auto handler = m_commandHandler.lock())
		{
			handler->IncomingRealTimeControl(value1, value2, value3, value4);
		}
	}
}
