#include <cstdlib>
#include <iostream>
#include "database.h"
#include "web_server.h"

int main()
{
    const char *databaseUrl = std::getenv("DATABASE_URL");
    if (!databaseUrl)
    {
        std::cerr << "DATABASE_URL is required (use a Neon PostgreSQL connection string).\n";
        return 1;
    }

    const char *portValue = std::getenv("PORT");
    int port = portValue ? std::atoi(portValue) : 8080;

    try
    {
        Database database(databaseUrl);
        database.initialize();
        std::cout << "Boda Stage Manager listening on port " << port << "\n";
        WebServer server(database, port);
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Startup failed: " << e.what() << '\n';
        return 1;
    }
}
