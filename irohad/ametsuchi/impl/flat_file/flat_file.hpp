/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_FLAT_FILE_HPP
#define IROHA_FLAT_FILE_HPP

#include <boost/optional.hpp>
#include <string>

namespace iroha {

  namespace ametsuchi {

    /**
     * Solid storage interface
     */
    class FlatFile {
     public:
      /**
       * Type of storage key
       */
      using Identifier = uint32_t;

      static const uint32_t DIGIT_CAPACITY = 16;

      /**
       * Add entity with binary data
       * @param id - reference key
       * @param blob - data associated with key
       */
      virtual bool add(Identifier id, const std::vector<uint8_t> &blob) = 0;

      /**
       * Get data associated with
       * @param id - reference key
       * @return - blob, if exists
       */
      virtual boost::optional<std::vector<uint8_t>> get(
          Identifier id) const = 0;

      /**
       * @return folder of storage
       */
      virtual std::string directory() const = 0;

      /**
       * @return maximal not null key
       */
      virtual Identifier last_id() const = 0;

      virtual void dropAll() = 0;

      virtual ~FlatFile() = default;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_FLAT_FILE_HPP
