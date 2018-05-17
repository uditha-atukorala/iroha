/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROPOSAL_TEMPLATE_HPP
#define IROHA_PROPOSAL_TEMPLATE_HPP

#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/transaction.hpp"

#include <boost/range/numeric.hpp>
#include "common_objects/trivial_proto.hpp"

#include "block.pb.h"
#include "interfaces/common_objects/types.hpp"
#include "proposal.pb.h"
#include "utils/lazy_initializer.hpp"

#include "transaction.hpp"

namespace shared_model {
  namespace proto {

    template <typename ParentType>
    class ProposalTemplate final
        : public CopyableProto<ParentType,
                               iroha::protocol::Proposal,
                               ProposalTemplate<ParentType>> {
      template <class T>
      using w = detail::PolymorphicWrapper<T>;
      using TransactionContainer = std::vector<w<interface::Transaction>>;

      using CopyableProtoType = CopyableProto<ParentType,
                                              iroha::protocol::Proposal,
                                              ProposalTemplate<ParentType>>;

     public:
      template <class ProposalType>
      explicit ProposalTemplate(ProposalType &&proposal)
          : CopyableProtoType(std::forward<ProposalType>(proposal)) {}

      ProposalTemplate(const ProposalTemplate<ParentType> &o)
          : ProposalTemplate(o.proto_) {}

      ProposalTemplate(ProposalTemplate<ParentType> &&o) noexcept
          : ProposalTemplate(std::move(o.proto_)) {}

      const TransactionContainer &transactions() const override {
        return *transactions_;
      }

      interface::types::TimestampType createdTime() const override {
        return CopyableProtoType::proto_->created_time();
      }

      interface::types::HeightType height() const override {
        return CopyableProtoType::proto_->height();
      }

     private:
      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<TransactionContainer> transactions_{[this] {
        return boost::accumulate(CopyableProtoType::proto_->transactions(),
                                 TransactionContainer{},
                                 [](auto &&vec, const auto &tx) {
                                   vec.emplace_back(new proto::Transaction(tx));
                                   return std::forward<decltype(vec)>(vec);
                                 });
      }};
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROPOSAL_TEMPLATE_HPP
