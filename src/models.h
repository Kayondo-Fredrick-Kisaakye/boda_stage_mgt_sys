#pragma once
#include <string>

struct Driver
{
    long id = 0;
    std::string stageName;
    std::string name;
    std::string phone;
    std::string plate;
    bool isActive = true;
};

struct StageRecord
{
    long id = 0;
    long driverId = 0;
    std::string stageName;
    std::string status;
    std::string recordedAt;
};
