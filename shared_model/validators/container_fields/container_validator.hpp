/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/format.hpp>
#include "datetime/time.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/verified_proposal.hpp"
#include "utils/polymorphic_wrapper.hpp"
#include "validators/answer.hpp"
#include "validators/container_fields/height_validator.hpp"

#ifndef IROHA_CONTAINER_VALIDATOR_HPP
#define IROHA_CONTAINER_VALIDATOR_HPP

namespace shared_model {
  namespace validation {

    template <typename Iface, typename FieldValidator, TransactionsValidator>
    class ContainerValidator : public HeightValidator,
                               public TransactionsValidator {
     public:
      ContainerValidator(FieldValidator field_validator = FieldValidator())
          : field_validator_(field_validator),
            TransactionsValidator(
                typename TransactionsValidator::TransactionValidator(
                    field_validator_)) {}

      Answer validate(const Iface &cont, std::string reason_name) const {
        Answer answer;
        ReasonsGroupType reason;
        reason.first = reason_name;
        field_validator_.validateCreatedTime(reason, cont.createdTime());
        HeightValidator::validateHeight(reason, cont.height());
        TransactionsValidator::validateTransactions(reason,
                                                    cont.transactions());
        if (not reason.second.empty()) {
          answer.addReason(std::move(reason));
        }
        return answer;
      }

     private:
      FieldValidator field_validator_;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_CONTAINER_VALIDATOR_HPP
