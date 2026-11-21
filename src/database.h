#pragma once
#include <memory>
#include <string>
#include <vector>
#include <pqxx/pqxx>
#include "models.h"

class Database
{
public:
    explicit Database(const std::string &connectionString);
    void initialize();
    void addDriver(const Driver &driver);
    std::vector<Driver> drivers();
    void recordStageVisit(long driverId, const std::string &status);
    std::vector<StageRecord> records();

private:
    std::string connectionString_;
    pqxx::connection connection();
};
