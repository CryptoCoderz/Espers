// Copyright (c) 2021-2025 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef DEMIMODULE_H
#define DEMIMODULE_H

#include <string>
#include <cstring>
// Compatibility for some compiliers (linking)
#include <fstream>
// For vector handling
#include <vector>

extern std::vector<std::string> vecRegisteredDemiNodes;
extern std::vector<std::string> vecSeenDemiNodes;
std::string GetDemiConfigFile();
void BuildDemiConfigFile();
void ReadDemiConfigFile();
void FindRegisteredDemi(std::string peerReadAddr);
bool FindSeenDemi(std::string peerReadAddr);
void UpdateDemiConfigFile();
void UpdateRegisteredDemi(std::string peerReadAddr, bool fAddDemi, bool fHaveDemi);
void UpdateSeenDemi(std::string peerReadAddr, bool fAddDemi, bool fHaveDemi);
void ResetRegisteredDemi();
void ResetSeenDemi();
void InitializeDemiConfigFile();

extern bool fDemiFound;

#endif // DEMIMODULE_H
