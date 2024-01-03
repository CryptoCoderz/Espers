// Copyright (c) 2021-2024 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef DEMIMODULE_H
#define DEMIMODULE_H

#include <string>
#include <cstring>
// Compatibility for some compiliers (linking)
#include <fstream>

std::string GetDemiConfigFile();
void ReadDemiConfigFile(std::string peerReadAddr);

extern bool fDemiFound;

#endif // DEMIMODULE_H
