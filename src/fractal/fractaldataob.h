// Copyright (c) 2019-2021 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef FRACTALDATAOB_H
#define FRACTALDATAOB_H

#include <string>

// Primitives
extern int input_length;
extern int shifts;
extern int position;
extern std::string Obfuscated_String;
extern std::string Obfuscated_Combined_String;
extern std::string Word_Letter_Count[];

// Preliminary obfuscation proceedure
void character_obfuscation(std::string contract_input, std::string contract_alias);// TODO: Refactor contract_alias to be able to write later as we want more than just character obbing
// Determined obfuscation logic shifts
void obfuscation_shift();
// Setup the obfuscation engine
void priming(std::string contract_input, std::string contract_alias);
// Ignition of obfuscation engine
void ignition();
// Flameout at obfuscation threshold
void flameout();
// Reassemble a flameout
void reassembly();

#endif // FRACTALDATAOB_H
