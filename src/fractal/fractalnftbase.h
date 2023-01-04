// Copyright (c) 2020-2023 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef FRACTALNFTBASE_H
#define FRACTALNFTBASE_H

#include <string>
#include <vector>

extern int n;
extern std::string nftBASEOut_String;
extern bool iLOAD;
extern bool NFTBASE_run;
extern bool load_image(std::vector<unsigned char>& image, const std::string& filename, int& x, int& y);
extern void write_image(std::string passed_alias, int w, int h, int channels, unsigned char data[], int contract_type, int PNGdata);

void NFTBASEparse(std::string filename);

#endif // FRACTALNFTBASE_H
