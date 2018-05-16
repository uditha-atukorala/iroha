#include <benchmark/benchmark.h>
#include <string>

#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/unsigned_proto.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "datetime/time.hpp"
#include "framework/base_tx.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "validators/permissions.hpp"

using namespace integration_framework;
using namespace shared_model;

const std::string kUser = "user";
const std::string kUserId = kUser + "@test";
const std::string kAmount = "1.0";
const crypto::Keypair kAdminKeypair =
    crypto::DefaultCryptoAlgorithmType::generateKeypair();
const crypto::Keypair kUserKeypair =
    crypto::DefaultCryptoAlgorithmType::generateKeypair();

auto baseTx() {
  return TestUnsignedTransactionBuilder().creatorAccountId(kUserId).createdTime(
      iroha::time::now());
}

//

static void BM_AddAssetQuantity(benchmark::State &state) {
  const std::string kAsset = "coin#test";

  auto make_perms = [&] {
    return framework::createUserWithPerms(
        kUser,
        kUserKeypair.publicKey(),
        "role",
        {shared_model::permissions::can_add_asset_qty})
        .build()
        .signAndAddSignature(kAdminKeypair);
  };

  IntegrationTestFramework itf(100);
  itf.setInitialState(kAdminKeypair);

  for (int i = 0; i < 100; i++) {
    itf.sendTx(make_perms());
  }
  itf.skipProposal().skipBlock();

  //  auto transaction = ;
  // define main benchmark loop
  while (state.KeepRunning()) {
    // define the code to be tested

    auto make_base = [&]() {
      auto base = baseTx();
      for (int i = 0; i < 100; i++) {
        base = base.addAssetQuantity(kUserId, kAsset, kAmount);
      }
      return base.build()
          .signAndAddSignature(kUserKeypair);
    };

    for (int i = 0; i < 100; i++) {
      itf.sendTx(make_base());
    }
    itf.skipProposal().skipBlock();
  }
  itf.done();
}
// define benchmark
BENCHMARK(BM_AddAssetQuantity)->Unit(benchmark::kMillisecond);

/// That's all. More in documentation.

// don't forget to include this:
BENCHMARK_MAIN();
