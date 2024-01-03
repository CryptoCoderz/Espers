// Copyright (c) 2021-2024 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef DEMINET_H
#define DEMINET_H

#include "database/txdb.h"

bool fDemiPeerRelay(std::string peerAddr);
bool getDemiBlock(uint256 blockHash);

#endif // DEMINET_H
