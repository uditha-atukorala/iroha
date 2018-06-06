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

    using ConsensusCacheType =
        boost::lockfree::spsc_queue<shared_model::interface::BlockVariant,
                                    boost::lockfree::capacity<1> >;
  }
}  // namespace iroha

#endif  // IROHA_CONSENSUS_CACHE_HPP
