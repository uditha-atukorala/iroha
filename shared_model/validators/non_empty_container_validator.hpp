/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_NON_EMPTY_CONTAINER_VALIDATOR_HPP
#define IROHA_NON_EMPTY_CONTAINER_VALIDATOR_HPP

#include "validators/container_validator.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Class that validates blocks and proposal common fieds
     */
    template <typename Iface,
              typename FieldValidator,
              typename TransactionValidator>
    class NonEmptyContainerValidator
        : public ContainerValidator<Iface, FieldValidator> {
     protected:
      void validateTransaction(
          ReasonsGroupType &reason,
          const interface::Transaction &transaction) const {
        auto answer = transaction_validator_.validate(transaction);
        if (answer.hasErrors()) {
          auto message = (boost::format("Tx: %s") % answer.reason()).str();
          reason.second.push_back(message);
        }
      }
      void validateTransactions(
          ReasonsGroupType &reason,
          const interface::types::TransactionsCollectionType &transactions)
          const {
        for (const auto &tx : transactions) {
          validateTransaction(reason, *tx);
        }
      }

      Answer validate(const Iface &cont, std::string reason_name) const {
        Answer answer;
        ReasonsGroupType reason;
        reason.first = reason_name;
        field_validator_.validateCreatedTime(reason, cont.createdTime());
        ContainerValidator<Iface, FieldValidator>::validateHeight(
            reason, cont.height());
        validateTransactions(reason, cont.transactions());
        if (not reason.second.empty()) {
          answer.addReason(std::move(reason));
        }
        return answer;
      }

     public:
      NonEmptyContainerValidator(
          const TransactionValidator &transaction_validator =
              TransactionValidator(),
          const FieldValidator &field_validator = FieldValidator())
          : transaction_validator_(transaction_validator),
            field_validator_(field_validator) {}

     private:
      TransactionValidator transaction_validator_;
      FieldValidator field_validator_;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_NON_EMPTY_CONTAINER_VALIDATOR_HPP
