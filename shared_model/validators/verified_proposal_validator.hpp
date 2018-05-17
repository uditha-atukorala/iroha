/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_VERIFIED_PROPOSAL_VALIDATOR_HPP
#define IROHA_VERIFIED_PROPOSAL_VALIDATOR_HPP

#include <boost/format.hpp>
#include "datetime/time.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/verified_proposal.hpp"
#include "utils/polymorphic_wrapper.hpp"
#include "validators/answer.hpp"
#include "validators/container_fields/height_validator.hpp"
#include "validators/container_fields/transactions_validator.hpp"

namespace shared_model {
  namespace validation {

    template <typename FieldValidator, typename TransactionValidator>
    class VerifiedProposalValidator
        : public HeightValidator,
          public TransactionsValidator<TransactionValidator> {
     public:
      /**
       * Applies validation on block
       * @param block
       * @return Answer containing found error if any
       */
      Answer validate(
          const interface::VerifiedProposal &verified_proposal) const {
        Answer answer;
        ReasonsGroupType reason;
        reason.first = "VerifiedProposal";
        field_validator_.validateCreatedTime(reason,
                                             verified_proposal.createdTime());
        HeightValidator::validateHeight(reason, verified_proposal.height());
        TransactionsValidator<TransactionValidator>::validateTransactions(
            reason, verified_proposal.transactions());
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

#endif  // IROHA_VERIFIED_PROPOSAL_VALIDATOR_HPP
