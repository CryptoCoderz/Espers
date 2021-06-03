// Copyright (c) 2017-2021 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef FRACTALCONTRACT_H
#define FRACTALCONTRACT_H

#include <string>

extern std::string selected_contract_alias;
extern std::string set_raw_input_data;
extern std::string set_contract_data;
extern std::string set_fractal_SCRIPT;

void create_smartCONTRACT(std::string raw_input_data, std::string contract_alias, int contract_type);
void edit_smartCONTRACT(std::string contract_data, std::string contract_alias);
void open_smartCONTRACT(std::string contract_alias, int contract_type);
void remove_smartCONTRACT(std::string contract_data, std::string contract_alias);

#endif // FRACTALCONTRACT_H
