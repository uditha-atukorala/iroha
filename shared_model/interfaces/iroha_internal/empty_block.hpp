/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_SHARED_MODEL_EMPTY_BLOCK_HPP
#define IROHA_SHARED_MODEL_EMPTY_BLOCK_HPP

#include "interfaces/iroha_internal/parent_block.hpp"

namespace shared_model {
  namespace interface {

    class EmptyBlock : public ParentBlock {
     public:
#ifndef DISABLE_BACKWARD
      iroha::model::Block *makeOldModel() const override {
        iroha::model::Block *old_block = new iroha::model::Block();
        old_block->height = height();
        constexpr auto hash_size = iroha::model::Block::HashType::size();
        old_block->prev_hash =
            *iroha::hexstringToArray<hash_size>(prevHash().hex());
        old_block->txs_number = 0;
        old_block->created_ts = createdTime();
        old_block->hash = *iroha::hexstringToArray<hash_size>(hash().hex());
        std::for_each(
            signatures().begin(), signatures().end(), [&old_block](auto &sig) {
              std::unique_ptr<iroha::model::Signature> old_sig(
                  sig.makeOldModel());
              old_block->sigs.emplace_back(*old_sig);
            });
        return old_block;
      }
#endif

    std::string toString() const override {
      return detail::PrettyStringBuilder()
          .init("Block")
          .append("hash", hash().hex())
          .append("height", std::to_string(height()))
          .append("prevHash", prevHash().hex())
          .append("createdtime", std::to_string(createdTime()))
          .append("signatures")
          .appendAll(signatures(), [](auto &sig) { return sig.toString(); })
          .finalize();
    }

    };

  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_EMPTY_BLOCK_HPP
