/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_sequence.hpp"

namespace shared_model {
  namespace interface {

    iroha::expected::Result<TransactionSequence, std::string>
    TransactionSequence::createTransactionSequence(
        const types::TransactionForwardCollectionType &transactions,
        const validation::TransactionsCollectionValidator &validator) {
      auto answer = validator.validate(transactions);
      if (answer.hasErrors()) {
        return iroha::expected::makeError(answer.reason());
      }
      return iroha::expected::makeValue(TransactionSequence(transactions));
    }

    types::TransactionForwardCollectionType
    TransactionSequence::transactions() {
      return transactions_;
    }

    TransactionSequence::TransactionSequence(
        const types::TransactionForwardCollectionType &transactions)
        : transactions_(transactions) {}

  }  // namespace interface
}  // namespace shared_model
