/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#include "ametsuchi/impl/temporary_wsv_impl.hpp"

#include "ametsuchi/impl/postgres_wsv_command.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "amount/amount.hpp"

namespace iroha {
  namespace ametsuchi {
    TemporaryWsvImpl::TemporaryWsvImpl(
        std::unique_ptr<pqxx::lazyconnection> connection,
        std::unique_ptr<pqxx::nontransaction> transaction)
        : connection_(std::move(connection)),
          transaction_(std::move(transaction)),
          wsv_(std::make_unique<PostgresWsvQuery>(*transaction_)),
          executor_(std::make_unique<PostgresWsvCommand>(*transaction_)),
          log_(logger::log("TemporaryWSV")) {
      auto query = std::make_shared<PostgresWsvQuery>(*transaction_);
      auto command = std::make_shared<PostgresWsvCommand>(*transaction_);
      command_executor_ = std::make_shared<CommandExecutor>(query, command);
      command_validator_ = std::make_shared<CommandValidator>(query);
      transaction_->exec("BEGIN;");
    }

    expected::Result<void, std::string> TemporaryWsvImpl::apply(
        const shared_model::interface::Transaction &tx,
        std::function<expected::Result<void, std::string>(
            const shared_model::interface::Transaction &, WsvQuery &)>
            apply_function) {
      const auto &tx_creator = tx.creatorAccountId();
      command_executor_->setCreatorAccountId(tx_creator);
      command_validator_->setCreatorAccountId(tx_creator);
      auto execute_command =
          [this, &tx_creator](
              auto &command,
              int command_index) -> expected::Result<void, std::string> {
        auto account = wsv_->getAccount(tx_creator).value();
        // Validate and execute command
        return boost::apply_visitor(*command_validator_, command.get())
                   .match(
                       [](expected::Value<void> &)
                           -> expected::Result<void, std::string> {
                         return {};
                       },
                       [command_index](expected::Error<CommandError> &error) -> expected::Result<void, std::string> {
                         return expected::makeError(
                             ((boost::format("stateful validation error: could "
                                             "not validate "
                                             "command with index %d: %s")
                               % command_index % error.error.toString()))
                                 .str());
                       })
            |
            [this, command_index, &command] {
              return boost::apply_visitor(*command_executor_, command.get())
                  .match(
                      [](expected::Value<void> &)
                          -> expected::Result<void, std::string> { return {}; },
                      [command_index](expected::Error<CommandError> &e) -> expected::Result<void, std::string> {
                        return expected::makeError(
                            ((boost::format(
                                  "stateful validation error: could not "
                                  "execute command with index %d: %s")
                              % command_index % e.error.toString()))
                                .str());
                      });
            };
      };
      transaction_->exec("SAVEPOINT savepoint_;");

      auto tx_failed = false;
      std::string errors_log = "";
      auto failed_cmd_processor =
          [&errors_log, &tx_failed](expected::Error<std::string> error) {
            errors_log += error.error + '\n';
            tx_failed = false;
          };

      const auto &commands = tx.commands();
      for (size_t i = 0; i < commands.size(); ++i) {
        execute_command(commands[i], i)
            .match([](expected::Value<void>) {}, failed_cmd_processor);
      };
      apply_function(tx, *wsv_).match([](expected::Value<void>) {},
                                      failed_cmd_processor);

      if (tx_failed) {
        transaction_->exec("RELEASE SAVEPOINT savepoint_;");
        return expected::makeError(errors_log);
      } else {
        transaction_->exec("ROLLBACK TO SAVEPOINT savepoint_;");
        return {};
      }
    }

    TemporaryWsvImpl::~TemporaryWsvImpl() {
      transaction_->exec("ROLLBACK;");
    }
  }  // namespace ametsuchi
}  // namespace iroha
