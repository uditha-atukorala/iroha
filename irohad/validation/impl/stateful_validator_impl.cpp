/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "validation/impl/stateful_validator_impl.hpp"

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include "builders/protobuf/proposal.hpp"
#include "common/result.hpp"
#include "validation/utils.hpp"

namespace iroha {
  namespace validation {

    StatefulValidatorImpl::StatefulValidatorImpl() {
      log_ = logger::log("SFV");
    }

    StatefulValidatorImpl::ProposalAndErrors StatefulValidatorImpl::validate(
        const shared_model::interface::Proposal &proposal,
        ametsuchi::TemporaryWsv &temporaryWsv) {
      log_->info("transactions in proposal: {}",
                 proposal.transactions().size());
      auto checking_transaction = [](const auto &tx, auto &queries) {
        return expected::Result<void, std::string>(
            [&]() -> expected::Result<void, std::string> {
              // Check if tx creator has account
              auto account = queries.getAccount(tx.creatorAccountId());
              if (account) {
                return expected::makeValue(*account);
              } else {
                return expected::makeError(
                    boost::format("stateful validator error: could not fetch "
                                  "account with id %d"
                                  % tx.creatorAccountId()));
              }
            } | [&](const auto &result) -> expected::Result<void, std::string> {
              // Check if account has quorum to execute transaction
              result.match(
                  [&](expected::Value<shared_model::interface::Account> account)
                      -> expected::Result<void, std::string> {
                    return boost::size(tx.signatures())
                            >= account.value.quorum()
                        ? expected::makeValue(
                              queries.getSignatories(tx.creatorAccountId()))
                        : expected::makeError(boost::format(
                              "stateful validator error: not enough "
                              "signatures; account's quorum %d, transaction's "
                              "signatures amount %d"s
                              % account.value.quorum()
                              % boost::size(tx.signatures())));
                  },
                  [](expected::Error<std::string> error) { return error; });
            } | [&](const auto &result) -> expected::Result<void, std::string> {
              // Check if signatures in transaction are account
              // signatory
              result.match(
                  [&](expected::Value<
                      std::vector<shared_model::interface::types::PubkeyType>>
                          signatories) {
                    signaturesSubset(tx.signatures(), signatories)
                        ? {}
                        : expected::makeError(formSignaturesErrorMsg(
                              tx.signatures(), signatories.value));
                  },
                  [](expected::Error<std::string> error) { return error; });
            });
      };

      // Filter only valid transactions and accumulate errors
      auto errors_log = ""s;
      auto filter = [&temporaryWsv, checking_transaction](auto &tx) {
        return temporaryWsv.apply(tx, checking_transaction)
            .match([](expected::Value<void>) { return true; },
                   [&errors_log](expected::Error<std::string> error) {
                     errors_log += error.error + "\n"s;
                     return false;
                   });
      };

      // TODO: kamilsa IR-1010 20.02.2018 rework validation logic, so that this
      // cast is not needed and stateful validator does not know about the
      // transport
      auto valid_proto_txs =
          proposal.transactions() | boost::adaptors::filtered(filter)
          | boost::adaptors::transformed([](auto &tx) {
              return static_cast<const shared_model::proto::Transaction &>(tx);
            });

      auto validated_proposal = shared_model::proto::ProposalBuilder()
                                    .createdTime(proposal.createdTime())
                                    .height(proposal.height())
                                    .transactions(valid_proto_txs)
                                    .createdTime(proposal.createdTime())
                                    .build();

      log_->info("transactions in verified proposal: {}",
                 validated_proposal.transactions().size());
      return std::make_pair(std::make_shared<decltype(validated_proposal)>(
                                validated_proposal.getTransport()),
                            errors_log);
    }

    std::string StatefulValidatorImpl::formSignaturesErrorMsg(
        const shared_model::interface::types::SignatureRangeType &signatures,
        const std::vector<shared_model::interface::types::PubkeyType>
            &signatories) {
      auto signatures_string = ""s, signatories_string = ""s;
      for (const auto &signature : signatures) {
        signatures_string += signature.publicKey() + "\n"s;
      }
      for (const auto &signatory : signatories) {
        signatories_string += signatory.toString() + "\n"s;
      }
      return boost::format(
                 "stateful validator error: signatures in transaction are not "
                 "account signatories:\n"
                 "signatures' public keys: %s\n"
                 "signatories: %s"
                 % signatures_string % signatories_string)
          .str();
    }
  }  // namespace validation
}  // namespace iroha
