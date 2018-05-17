/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_VERIFIED_PROPOSAL_HPP
#define IROHA_VERIFIED_PROPOSAL_HPP

#include "backend/protobuf/proposal_template.hpp"
#include "interfaces/iroha_internal/verified_proposal.hpp"

namespace shared_model {
  namespace proto {
    using VerifiedProposal = ProposalTemplate<interface::VerifiedProposal>;
  }
}  // namespace shared_model

#endif  // IROHA_VERIFIED_PROPOSAL_HPP
