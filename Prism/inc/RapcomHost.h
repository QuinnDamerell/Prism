#pragma once

#include <string>

#include "Common.h"

#include "RapcomBase.h"
#include "IRapcomListener.h"
#include "SharedFromThisHelper.h"
#include "IControlCommandHandler.h"
#include "rapidjson/document.h"

DECLARE_SMARTPOINTER(RapcomHost);
class RapcomHost :
    public LightFx::SharedFromThis,
    public Rapcom::IRapcomListener
{
public:
    RapcomHost(IControlCommandHandlerWeakPtr commandHandler);

    // Must be called to initialize the object
    void Initialize();

    // Gets the config.
    rapidjson::Document& GetConfig() { return m_rapcom->GetConfig(); }

    // Saves the config
    void SaveConfig() { m_rapcom->SaveConfig(); }

    //
    // IRapcomListener

    // Fired when the config has been changed.
    virtual void OnConfigChange(rapidjson::Document& oldConfig, rapidjson::Document& newConfig);

    // Fired when a command has been issued
    virtual void OnCommand(rapidjson::Document& command);

private:
    // Holds the callback reference
    IControlCommandHandlerWeakPtr m_commandHandler;

    // The rapcom object
    Rapcom::RapcomBasePtr m_rapcom;

    // Sets an error to the document
    void SetDocumentError(rapidjson::Document& document, std::string errorText);

    // Sets success on a doc
    void SetDocumentSuccess(rapidjson::Document& document);
};