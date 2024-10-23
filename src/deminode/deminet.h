// Copyright (c) 2021-2024 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef DEMINET_H
#define DEMINET_H

#include "database/txdb.h"

bool fDemiPeerRelay(std::string peerAddr);
void DemiDataRequestHandler(std::string peerAddr, bool fDemiFetched, bool fDemiSingleShot, const CInv dinv);
void DemiFetchBlock(uint256 blockHash);
void DemiFetchLatest(bool fDemiFetched);
void DemiFetchMulti(uint256 blockHash, std::string peerAddr, bool fDemiFetched);
bool getDemiBlock(uint256 blockHash);
void DemiDataRefresh();
void DemiProcessRefresh();
void DemiConsensusLogix();
bool getDemiNetStatus(int nTotalDemi, int nCurrentDemi);
bool getDemiNodeInfo(std::vector<std::string> sDemiPeers, std::vector<int64_t> nDemiHeights, std::vector<uint256> vecDemiHashes);

#endif // DEMINET_H
