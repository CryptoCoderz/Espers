// Copyright (c) 2017-2023 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef FRACTALENGINE_H
#define FRACTALENGINE_H

#include <string>

void write_contractDATA(std::string obfuscated_write_string, std::string contract_alias, int contract_type);
void read_contractDATA(std::string contract_alias, int contract_type);
void parse_fractalSCRIPT(std::string fractal_SCRIPT);

#endif // FRACTALENGINE_H
