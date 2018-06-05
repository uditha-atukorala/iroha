/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/queries/blocks_query.hpp"
#include "interfaces/base/model_primitive.hpp"
#include <boost/variant.hpp>

namespace shared_model {
  namespace interface {

    std::string BlocksQuery::toString() const {
      return detail::PrettyStringBuilder()
          .init("BlocksQuery")
          .append("creatorId", creatorAccountId())
          .append("queryCounter", std::to_string(queryCounter()))
          .append(Signable::toString())
          .finalize();
    }

    bool BlocksQuery::operator==(const ModelType &rhs) const {
      return creatorAccountId() == rhs.creatorAccountId()
          && queryCounter() == rhs.queryCounter()
          && createdTime() == rhs.createdTime()
          && signatures() == rhs.signatures();
    }

  }  // namespace interface
}  // namespace shared_model
