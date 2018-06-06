/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_CONSENSUS_CACHE_HPP
#define IROHA_CONSENSUS_CACHE_HPP

#include <boost/lockfree/spsc_queue.hpp>
#include "interfaces/iroha_internal/block_variant.hpp"

namespace iroha {
  namespace network {

    class ConsensusCacheType {
     private:
      mutable std::mutex _mtx;
      std::shared_ptr<shared_model::interface::BlockVariant> block_variant_;

     public:
      std::shared_ptr<shared_model::interface::BlockVariant> get() const {
        std::lock_guard<std::mutex> l(_mtx);
        return block_variant_;
      }  // auto unlock (lock_guard, RAII)

      void insert(
          std::shared_ptr<shared_model::interface::BlockVariant> value) {
        std::lock_guard<std::mutex> l(_mtx);
        block_variant_ = value;
      }  // auto unlock (lock_guard, RAII)
    };
  }  // namespace network
}  // namespace iroha

#endif  // IROHA_CONSENSUS_CACHE_HPP
