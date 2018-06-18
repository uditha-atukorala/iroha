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

#ifndef IROHA_POSTGRES_COMMAND_EXECUTOR_HPP
#define IROHA_POSTGRES_COMMAND_EXECUTOR_HPP

namespace iroha {
  namespace ametsuchi {
    std::string add_extansion =
        ""
        "CREATE EXTENSION IF NOT EXISTS plpython3u;";

    std::string add_asset_quantity_command =
        ""
        "CREATE OR REPLACE FUNCTION AddAssetQuantity("
        "           account_id varchar, "
        "           asset_id varchar, "
        "           amount_value varchar, "
        "           amount_precision integer)"
        "    RETURNS integer "
        "AS $$"
        " rs = plpy.execute(\"SELECT * FROM asset WHERE asset_id = '\" + asset_id + \"'\")\n"
        " if rs.nrows() == 0:\n"
        "   return 1\n"
        " if rs[0][\"precision\"] != amount_precision:\n"
        "   return 2\n"
        " rs = plpy.execute(\"SELECT count(*) AS c FROM account WHERE account_id = '\" + account_id + \"'\")\n"
        " if rs[0][\"c\"] == 0:\n"
        "   return 3\n"
        " rs_account_asset = plpy.execute(\"SELECT * FROM account_has_asset WHERE account_id = '\" + account_id + \"' AND asset_id = '\" + asset_id + \"'\")\n"
        " new_balance = float(amount_value)\n"
        " if rs_account_asset.nrows() != 0:\n"
        "   new_balance += float(rs_account_asset[0][\"amount\"])\n"
        "   rs = plpy.execute(\"UPDATE account_has_asset SET amount = '\" + str(new_balance) + \"' WHERE account_id = '\" + account_id + \"' AND asset_id = '\" + asset_id + \"'\")\n"
        " else:\n"
        "   rs = plpy.execute(\"INSERT INTO account_has_asset(account_id, asset_id, amount) VALUES ('\" + account_id + \"', '\" + asset_id + \"', '\" + str(new_balance) + \"')\")\n"
        "   return 10\n"
        " return 0\n"
        "$$ LANGUAGE plpython3u";

    std::string get_account_query =
        ""
        "CREATE OR REPLACE FUNCTION GetAccount("
            "account_id_value varchar)"
            " RETURNS TABLE (account_id varchar, domain_id varchar, quorum int, data JSONB) AS $$\n"
            " SELECT * FROM account WHERE account_id = account_id_value;\n"
            " $$ LANGUAGE sql;";
  }  // namespace ametsuchi
}  // namespace iroha
#endif  // IROHA_POSTGRES_COMMAND_EXECUTOR_HPP
