/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <benchmark/benchmark.h>
#include <pqxx/transaction>
#include <pqxx/nontransaction>
#include <pqxx/pqxx>
#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>

#define OTL_ODBC_UNIX
#define OTL_ODBC_ALTERNATE_RPC
#define OTL_ODBC
#include "otlv4.h"

std::string opt = "host=localhost port=5432 user=postgres password=mysecretpassword";
std::string query = "select * from peer;";

class LibpqxxBenchmark : public benchmark::Fixture {
 public:
  void SetUp(const ::benchmark::State &state) {
    postgres_connection = std::make_unique<pqxx::lazyconnection>(opt);
    try {
      postgres_connection->activate();
    } catch (const pqxx::broken_connection &e) {
      assert(false);
    }

    transaction =
        std::make_unique<pqxx::nontransaction>(*postgres_connection, "pqxx benchmark");

    transaction->exec("CREATE TABLE IF NOT EXISTS peer ("
                  "    public_key bytea NOT NULL,"
                  "    address character varying(261) NOT NULL UNIQUE,"
                  "    PRIMARY KEY (public_key)"
                  ");");
  }

  void TearDown(const ::benchmark::State &state) {
    transaction->exec("DROP TABLE peer;");
    transaction.release();
  }
  std::unique_ptr<pqxx::nontransaction> transaction;
  std::unique_ptr<pqxx::lazyconnection> postgres_connection;
};

BENCHMARK_F(LibpqxxBenchmark, GetPeers)(benchmark::State &state) {
  while (state.KeepRunning()) {
    transaction->exec(query);
  }
}

class SOCIBenchmark : public benchmark::Fixture {
 public:
  void SetUp(const ::benchmark::State &state) {
    session = std::make_unique<soci::session>(soci::postgresql, opt);


    *session << "CREATE TABLE IF NOT EXISTS peer ("
                          "    public_key bytea NOT NULL,"
                          "    address character varying(261) NOT NULL UNIQUE,"
                          "    PRIMARY KEY (public_key)"
                          ");";
  }

  void TearDown(const ::benchmark::State &state) {
    *session << "DROP TABLE peer;";
  }
  std::unique_ptr<soci::session> session;
};

BENCHMARK_F(SOCIBenchmark, GetPeers)(benchmark::State &state) {
  while (state.KeepRunning()) {
    *session << query;
  }
}

class OTLBenchmark : public benchmark::Fixture {
 public:
  void SetUp(const ::benchmark::State &state) {
    otl_connect::otl_initialize();

    db = std::make_unique<otl_connect>();
    db->rlogon("postgres/mysecretpassword@postgresql");

  }

  void TearDown(const ::benchmark::State &state) {
    db->logoff();
  }
  std::unique_ptr<otl_connect> db;
};

BENCHMARK_F(OTLBenchmark, GetPeers)(benchmark::State &state) {
  while (state.KeepRunning()) {
  }
}

BENCHMARK_MAIN();
