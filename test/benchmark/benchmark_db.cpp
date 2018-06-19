/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <benchmark/benchmark.h>
#include <soci/postgresql/soci-postgresql.h>
#include <soci/soci.h>
#include <pqxx/nontransaction>
#include <pqxx/pqxx>
#include <pqxx/transaction>

int commands = 5;
int transactions = 100;
int blocks = 10;

std::string opt =
    "host=localhost port=5432 user=postgres password=mysecretpassword";
std::string query = "select * from peer;";

static void BM_Transaction(benchmark::State &state) {
  pqxx::connection conn(opt);
  pqxx::work tx(conn);
  tx.exec("DROP TABLE IF EXISTS account");
  tx.exec(
      "CREATE TABLE  account ("
      "name text UNIQUE, "
      "balance INT"
      ");");
  tx.exec(
      "CREATE INDEX  namehash ON account USING hash(name)"
      );
  tx.exec("INSERT INTO account VALUES ('a', 0)");
  tx.commit();
  // define main benchmark loop
  int count = 0;
  while (state.KeepRunning()) {
    int balance = 0;
    for (int k = 0; k < blocks; k++) {
      pqxx::work tx3(conn);
      std::string txstr;
      for (int j = 0; j < transactions; j++) {
        for (int i = 0; i < commands; i++) {

          tx3.exec("SELECT * FROM account WHERE name = 'a' LIMIT 1;\n");
//          tx3.exec("UPDATE  account SET balance = " + std::to_string(balance + i)
//              + " WHERE name = 'a';\n");
          tx3.exec("INSERT INTO account values('a', " + std::to_string(balance + i) + ") ON CONFLICT (name) DO UPDATE SET balance = EXCLUDED.balance;\n");
          tx3.exec("INSERT INTO account VALUES ('" + std::to_string(count) + "', 0);\n");
          count++;
//          tx3.exec(txstr);
//          txstr = "";
        }
      }

      tx3.commit();
    }
  }
  pqxx::work tx2(conn);
  tx2.exec("DROP INDEX namehash");
  tx2.exec("DROP TABLE account");
  tx2.commit();
}

static void BM_NonTransaction(benchmark::State &state) {
  pqxx::connection conn(opt);
  pqxx::nontransaction tx(conn);
  tx.exec(
      "CREATE TABLE IF NOT EXISTS account ("
      "name text, "
      "balance INT"
      ");");
  tx.exec("INSERT INTO account VALUES ('a', 0)");
  tx.commit();
  // define main benchmark loop
  while (state.KeepRunning()) {
    int balance = 0;
    for (int k = 0; k < blocks; k++) {
      pqxx::nontransaction tx3(conn);
      for (int j = 0; j < transactions; j++) {
        for (int i = 0; i < commands; i++) {
          auto result = tx3.exec("SELECT * FROM account WHERE name = 'a'");
          tx3.exec("UPDATE  account SET balance = " + std::to_string(balance + i)
                      + "WHERE name = 'a'");
          tx3.exec(
              "INSERT INTO account VALUES ('" + std::to_string(i) + "', 0)");
          balance = result.at(0).at("balance").template as<int>();
        }
      }
      tx3.commit();
    }
  }
  pqxx::nontransaction tx2(conn);
  tx2.exec("DROP TABLE account");
  tx2.commit();
}

static void BM_NonTransactionManual(benchmark::State &state) {
  pqxx::connection conn(opt);
  pqxx::nontransaction tx(conn);
  tx.exec("begin");
  tx.exec(
      "CREATE TABLE IF NOT EXISTS account ("
      "name text, "
      "balance INT"
      ");");
  tx.exec("INSERT INTO account VALUES ('a', 0)");
  tx.exec("commit");
  tx.commit();
  // define main benchmark loop

  while (state.KeepRunning()) {
    int balance = 0;
    for (int k = 0; k < blocks; k++) {
      pqxx::nontransaction tx3(conn);
      tx3.exec("begin");
      for (int j = 0; j < transactions; j++) {
        for (int i = 0; i < commands; i++) {
          auto result = tx3.exec("SELECT * FROM account WHERE name = 'a'");
          tx3.exec(
              "UPDATE  account SET balance = " + std::to_string(balance + i)
                  + "WHERE name = 'a'");
          tx3.exec(
              "INSERT INTO account VALUES ('" + std::to_string(i) + "', 0)");
          balance = result.at(0).at("balance").template as<int>();
        }
      }
      tx3.exec("commit");
      tx3.commit();
    }
  }

  pqxx::nontransaction tx2(conn);
  tx2.exec("DROP TABLE account");
  tx2.commit();
//  tx2.exec("commit");
}


static void BM_Prepared(benchmark::State &state) {
  pqxx::connection conn(opt);
  conn.prepare(
      "update",
      "UPDATE account SET balance = $1 WHERE name = 'a'");
  conn.prepare(
      "insert",
      "INSERT INTO account VALUES ($1, 0)"
      );
  pqxx::work tx(conn);
  tx.exec(
      "CREATE TABLE IF NOT EXISTS account ("
      "name text, "
      "balance INT"
      ");");

  tx.exec("INSERT INTO account VALUES ('a', 0)");
  tx.commit();
  // define main benchmark loop
  while (state.KeepRunning()) {
    int balance = 0;
    for (int k = 0; k < blocks; k++) {
      pqxx::work tx(conn);
      for (int j = 0; j < transactions; j++) {
        for (int i = 0; i < commands; i++) {
          auto result = tx.exec("SELECT * FROM account WHERE name = 'a'");
          tx.exec_prepared("update", balance + i);
          tx.exec_prepared("insert", std::to_string(i));
          balance = result.at(0).at("balance").template as<int>();
        }
      }
      tx.commit();
    }
  }
  pqxx::work tx2(conn);
  tx2.exec("DROP TABLE account");
  tx2.commit();
}

static void BM_NonTransactionManualPrepared(benchmark::State &state) {
  pqxx::connection conn(opt);
  pqxx::nontransaction tx(conn);
  conn.prepare(
      "update",
      "UPDATE account SET balance = $1 WHERE name = 'a'");
  conn.prepare(
      "insert",
      "INSERT INTO account VALUES ($1, 0)"
  );

  tx.exec("begin");
  tx.exec(
      "CREATE TABLE IF NOT EXISTS account ("
      "name text, "
      "balance INT"
      ");");
  tx.exec("INSERT INTO account VALUES ('a', 0)");
  tx.exec("commit");
  tx.commit();
  // define main benchmark loop

  while (state.KeepRunning()) {
    int balance = 0;
    for (int k = 0; k < blocks; k++) {
      pqxx::nontransaction tx3(conn);
      tx3.exec("begin");
      for (int j = 0; j < transactions; j++) {
        for (int i = 0; i < commands; i++) {
          auto result = tx3.exec("SELECT * FROM account WHERE name = 'a'");
          tx3.exec_prepared("update", balance + i);
          tx3.exec_prepared("insert", std::to_string(i));
          balance = result.at(0).at("balance").template as<int>();
        }
      }
      tx3.exec("commit");
      tx3.commit();
    }
  }
  pqxx::work tx2(conn);
  tx2.exec("DROP TABLE account");
  tx2.commit();
}

static void BM_SociSession(benchmark::State &state) {
  soci::session session(soci::postgresql, opt);

  session << "CREATE TABLE IF NOT EXISTS account ("
             "name text, "
             "balance INT"
             ");";
  session << "INSERT INTO account VALUES ('a', 0)";

  // define main benchmark loop
  while (state.KeepRunning()) {
    int balance = 0;
    for (int k = 0; k < blocks; k++) {
      for (int j = 0; j < transactions; j++) {
        for (int i = 0; i < commands; i++) {
          session << "SELECT balance FROM account WHERE name = 'a'", soci::into(
              balance);
          session << "UPDATE  account SET balance = "
                  << std::to_string(balance + i)
                  << "WHERE name = 'a'";
          session << "INSERT INTO account VALUES ('" << std::to_string(i)
                  << "', 0)";
        }
      }
    }
  }
  session << "DROP TABLE account";
}

static void BM_SociTransaction(benchmark::State &state) {
  soci::session session(soci::postgresql, opt);

  session << "CREATE TABLE IF NOT EXISTS account ("
             "name text, "
             "balance INT"
             ");";
  session << "INSERT INTO account VALUES ('a', 0)";

  // define main benchmark loop
  while (state.KeepRunning()) {

    int balance = 0;
    for (int k = 0; k < blocks; k++) {
      soci::transaction tx(session);
      for (int j = 0; j < transactions; j++) {
        for (int i = 0; i < commands; i++) {
          session << "SELECT balance FROM account WHERE name = 'a'", soci::into(
              balance);
          session << "UPDATE  account SET balance = "
                  << std::to_string(balance + i)
                  << "WHERE name = 'a'";
          session << "INSERT INTO account VALUES ('" << std::to_string(i)
                  << "', 0)";
        }
      }
      tx.commit();
    }
  }
  session << "DROP TABLE account";
}

// define benchmark
BENCHMARK(BM_Transaction)->Unit(benchmark::kMillisecond);
//BENCHMARK(BM_NonTransaction)->Unit(benchmark::kMillisecond);
//BENCHMARK(BM_Prepared)->Unit(benchmark::kMillisecond);
//BENCHMARK(BM_NonTransactionManual)->Unit(benchmark::kMillisecond);
//BENCHMARK(BM_NonTransactionManualPrepared)->Unit(benchmark::kMillisecond);
//BENCHMARK(BM_SociSession)->Unit(benchmark::kMillisecond);
//BENCHMARK(BM_SociTransaction)->Unit(benchmark::kMillisecond);

//class LibpqxxBenchmark : public benchmark::Fixture {
// public:
//  void SetUp(const ::benchmark::State &state) {
//    postgres_connection = std::make_unique<pqxx::lazyconnection>(opt);
//    try {
//      postgres_connection->activate();
//    } catch (const pqxx::broken_connection &e) {
//      assert(false);
//    }
//
//    transaction = std::make_unique<pqxx::nontransaction>(*postgres_connection,
//                                                         "pqxx benchmark");
//
//    transaction->exec(
//        "CREATE TABLE IF NOT EXISTS peer ("
//        "    public_key bytea NOT NULL,"
//        "    address character varying(261) NOT NULL UNIQUE,"
//        "    PRIMARY KEY (public_key)"
//        ");");
//  }
//
//  void TearDown(const ::benchmark::State &state) {
//    transaction->exec("DROP TABLE peer;");
//    transaction.release();
//  }
//  std::unique_ptr<pqxx::nontransaction> transaction;
//  std::unique_ptr<pqxx::lazyconnection> postgres_connection;
//};
//
//BENCHMARK_F(LibpqxxBenchmark, GetPeers)(benchmark::State &state) {
//  while (state.KeepRunning()) {
//    transaction->exec(query);
//  }
//}
//
//class SOCIBenchmark : public benchmark::Fixture {
// public:
//  void SetUp(const ::benchmark::State &state) {
//    session = std::make_unique<soci::session>(soci::postgresql, opt);
//
//    *session << "CREATE TABLE IF NOT EXISTS peer ("
//                "    public_key bytea NOT NULL,"
//                "    address character varying(261) NOT NULL UNIQUE,"
//                "    PRIMARY KEY (public_key)"
//                ");";
//  }
//
//  void TearDown(const ::benchmark::State &state) {
//    *session << "DROP TABLE peer;";
//  }
//  std::unique_ptr<soci::session> session;
//};
//
//BENCHMARK_F(SOCIBenchmark, GetPeers)(benchmark::State &state) {
//  while (state.KeepRunning()) {
//  }
//}
//
//BENCHMARK_F(OTLBenchmark, GetPeers)(benchmark::State &state) {
//  while (state.KeepRunning()) {
//  }
//}

BENCHMARK_MAIN();
