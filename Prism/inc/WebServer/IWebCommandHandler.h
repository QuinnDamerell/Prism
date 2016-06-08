#pragma once

#include "Common.h"

#include "mongoose.h"

DECLARE_SMARTPOINTER(IWebCommandHandler);
class IWebCommandHandler 
{
public:
    virtual void HandleWebCall(struct mg_connection *nc, int ev, void *ev_data) = 0;
};