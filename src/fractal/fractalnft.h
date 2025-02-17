// Copyright (c) 2020-2025 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef FRACTALNFT_H
#define FRACTALNFT_H

#include <string>

extern std::string NFTdata;
extern bool NFT_run;

void NFTrender(std::string input_mydata, std::string input_alias, std::string render_alias, int contract_type);
void NFTprintRENDER(unsigned char NFTdata[], int w, int h, int channels, std::string passed_alias, int contract_type);
void NFTparse(std::string image_to_deCode);
void NFTenCode(std::string input_thatdata);

#endif // FRACTALNFT_H
