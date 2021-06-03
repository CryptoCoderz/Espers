// Copyright (c) 2020-2021 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef FRACTALNFT_H
#define FRACTALNFT_H

#include <string>
#include <vector>

extern int n;
extern std::string nftOut_String;
extern bool iLOAD;
extern bool NFT_run;
extern bool load_image(std::vector<unsigned char>& image, const std::string& filename, int& x, int& y);

void NFTparse(std::string filename);

#endif // FRACTALNFT_H
