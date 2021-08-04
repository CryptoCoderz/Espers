// Copyright (c) 2020-2021 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef FRACTALBVAC_H
#define FRACTALBVAC_H

#include <string>

extern std::string thatdata;
extern bool BVAC_run;

void enCode(std::string input_mydata, std::string input_alias);
void printCODED(int char_TOTAL, int w, int h, int channels, std::string passed_alias);
void deCode(std::string image_to_deCode);
void match_deCode(std::string input_thatdata);

#endif // FRACTALBVAC_H
