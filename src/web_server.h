#pragma once
#include <memory>
#include "database.h"

class WebServer
{
public:
    WebServer(Database &database, int port);
    void run();

private:
    Database &database_;
    int port_;
};
