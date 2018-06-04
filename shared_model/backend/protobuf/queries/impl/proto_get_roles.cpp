/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/queries/proto_get_roles.hpp"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
    namespace proto {

        template <typename QueryType>
        GetRoles::GetRoles(QueryType &&query)
        : CopyableProto(std::forward<QueryType>(query)) {}

        GetRoles::GetRoles(const GetRoles &o) : GetRoles(o.proto_) {}

        GetRoles::GetRoles(GetRoles &&o) noexcept : GetRoles(std::move(o.proto_)) {}

    }
}
