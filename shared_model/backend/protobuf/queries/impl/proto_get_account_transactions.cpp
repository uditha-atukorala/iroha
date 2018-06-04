/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/queries/proto_get_account_transactions.hpp"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {

    template <typename QueryType>
    GetAccountTransactions::GetAccountTransactions(QueryType &&query)
        : CopyableProto(std::forward<QueryType>(query)),
          account_transactions_{proto_->payload().get_account_transactions()} {}

    GetAccountTransactions::GetAccountTransactions(
        const GetAccountTransactions &o)
        : GetAccountTransactions(o.proto_) {}

    GetAccountTransactions::GetAccountTransactions(
        GetAccountTransactions &&o) noexcept
        : GetAccountTransactions(std::move(o.proto_)) {}

    const interface::types::AccountIdType &GetAccountTransactions::accountId()
        const {
      return account_transactions_.account_id();
    }

  }  // namespace proto
}  // namespace shared_model
