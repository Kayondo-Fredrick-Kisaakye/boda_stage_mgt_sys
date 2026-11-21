#include "database.h"
#include <fstream>
#include <sstream>
#include <pqxx/pqxx>

Database::Database(const std::string &connectionString)
    : connectionString_(connectionString) {}

pqxx::connection Database::connection()
{
    return pqxx::connection(connectionString_);
}

void Database::initialize()
{
    auto db = connection();
    pqxx::work tx(db);
    tx.exec0(R"SQL(
    CREATE TABLE IF NOT EXISTS drivers (
      id BIGSERIAL PRIMARY KEY,
      stage_name VARCHAR(120) NOT NULL,
      name VARCHAR(120) NOT NULL,
      phone VARCHAR(30) NOT NULL,
      plate VARCHAR(30) NOT NULL UNIQUE,
      is_active BOOLEAN NOT NULL DEFAULT TRUE,
      joined_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
    );
    CREATE TABLE IF NOT EXISTS stage_records (
      id BIGSERIAL PRIMARY KEY,
      driver_id BIGINT NOT NULL REFERENCES drivers(id) ON DELETE CASCADE,
      stage_name VARCHAR(120) NOT NULL,
      status VARCHAR(20) NOT NULL CHECK (status IN ('checked_in', 'checked_out')),
      recorded_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
    );
  )SQL");
    tx.commit();
}

void Database::addDriver(const Driver &driver)
{
    auto db = connection();
    pqxx::work tx(db);
    tx.exec_params("INSERT INTO drivers(stage_name,name,phone,plate) VALUES($1,$2,$3,$4)",
                   driver.stageName, driver.name, driver.phone, driver.plate);
    tx.commit();
}

std::vector<Driver> Database::drivers()
{
    auto db = connection();
    pqxx::work tx(db);
    auto rows = tx.exec("SELECT id,stage_name,name,phone,plate,is_active FROM drivers ORDER BY id DESC");
    std::vector<Driver> result;
    for (const auto &row : rows)
    {
        Driver d;
        d.id = row[0].as<long>();
        d.stageName = row[1].as<std::string>();
        d.name = row[2].as<std::string>();
        d.phone = row[3].as<std::string>();
        d.plate = row[4].as<std::string>();
        d.isActive = row[5].as<bool>();
        result.push_back(d);
    }
    tx.commit();
    return result;
}

void Database::recordStageVisit(long driverId, const std::string &status)
{
    auto db = connection();
    pqxx::work tx(db);
    auto row = tx.exec_params("SELECT stage_name FROM drivers WHERE id=$1", driverId);
    if (row.empty())
        throw std::runtime_error("Driver not found");
    tx.exec_params("INSERT INTO stage_records(driver_id,stage_name,status) VALUES($1,$2,$3)",
                   driverId, row[0][0].as<std::string>(), status);
    tx.commit();
}

std::vector<StageRecord> Database::records()
{
    auto db = connection();
    pqxx::work tx(db);
    auto rows = tx.exec("SELECT id,driver_id,stage_name,status,to_char(recorded_at,'YYYY-MM-DD HH24:MI') FROM stage_records ORDER BY id DESC LIMIT 100");
    std::vector<StageRecord> result;
    for (const auto &row : rows)
    {
        StageRecord r;
        r.id = row[0].as<long>();
        r.driverId = row[1].as<long>();
        r.stageName = row[2].as<std::string>();
        r.status = row[3].as<std::string>();
        r.recordedAt = row[4].as<std::string>();
        result.push_back(r);
    }
    tx.commit();
    return result;
}
