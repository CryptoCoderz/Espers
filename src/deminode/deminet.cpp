// Copyright (c) 2021-2024 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "deminet.h"
#include "demimodule.h"

// Store Demi network voting log
std::vector<std::string> vecVoteDeminodes;
// Store Demi-node heights, hashes, and IPs
std::vector<std::string> vecFetchedDemiData;
// Store Demi-node last data request time
int64_t nDemiFetchIntervalNet;
int64_t nDemiFetchIntervalSolo;

bool fDemiPeerRelay(std::string peerAddr)
{
    // Search through Registered Demi-node list
    FindRegisteredDemi(peerAddr);
    if(fDemiFound) {
        // Return success
        LogPrintf("Demi-node System: fDemiPeerRelay - Peer: %s matches listed Demi-node!\n", peerAddr);
        return true;
    }
    return false;
}

void DemiFetchBlock(uint256 blockHash)
{
    // Node data Lock
    LOCK(cs_vNodes);
    // block allocation
    vector<CInv> vGetDemiData;
    const CInv& dinv = CInv(MSG_DEMIBLOCK, blockHash);
    vGetDemiData.push_back(dinv);
    // Reset previously set data
    fDemiFound = false;
    // Loop through peers/nodes
    for(CNode* pnode : vNodes) {
        // Skip obsolete nodes
        if(pnode->nVersion < DEMINODE_VERSION) {
            continue;
        }
        // Request block data if peer is Demi-node
        if(fDemiPeerRelay(pnode->addrName)) {
            LogPrintf("Demi-node System: DemiFetchBlock - Requesting block %s from %s\n", blockHash.ToString().c_str(), pnode->addrName.c_str());
            pnode->PushMessage("getdata", dinv);
        }
    }

    vGetDemiData.clear();
    if(fDemiFound) {
        LogPrintf("Demi-node System: DemiFetchBlock - Requested data successfully!\n");
    }
}

void DemiFetchLatest()
{
    // Request Demi-node current best-block
    //
    uint256 blockHash = Params().GenesisBlock().GetHash();
    // Node data Lock
    LOCK(cs_vNodes);
    // block allocation
    vector<CInv> vGetDemiData;
    const CInv& dinv = CInv(MSG_DEMICURRENT, blockHash);
    vGetDemiData.push_back(dinv);
    // Reset previously set data
    fDemiFound = false;
    // Loop through peers/nodes
    for(CNode* pnode : vNodes) {
        // Skip obsolete nodes
        if(pnode->nVersion < DEMINODE_VERSION) {
            continue;
        }
        // Request block data if peer is Demi-node
        if(fDemiPeerRelay(pnode->addrName)) {
            // LogPrintf("Demi-node System: DemiFetchLatest - Requesting block %s from %s\n", blockHash.ToString().c_str(), pnode->addrName.c_str());
            pnode->PushMessage("getdata", dinv);
        }

    }

    vGetDemiData.clear();
    if(fDemiFound) {
        LogPrintf("Demi-node System: DemiFetchLatest - Requested Latest Block data successfully!\n");
    }
}

void DemiFetchMulti(uint256 blockHash, std::string peerAddr)
{
    // Node data Lock
    LOCK(cs_vNodes);
    // block allocation
    vector<CInv> vGetDemiData;
    const CInv& dinv = CInv(MSG_DEMIMULTI, blockHash);
    vGetDemiData.push_back(dinv);
    // Reset previously set data
    fDemiFound = false;
    // Loop through peers/nodes
    for(CNode* pnode : vNodes) {
        // Skip obsolete nodes
        if(pnode->nVersion < DEMINODE_VERSION) {
            continue;
        }
        // Request block data from specified Demi-node
        if(pnode->addrName == peerAddr) {
            //LogPrintf("Demi-node System: DemiFetchBlock - Requesting block %s from %s\n", blockHash.ToString().c_str(), pnode->addrName.c_str());
            pnode->PushMessage("getdata", dinv);
            break;
        }
    }

    vGetDemiData.clear();
    if(fDemiFound) {
        LogPrintf("Demi-node System: DemiFetchMulti - Requested Block data chunk successfully!\n");
    }
}

bool getDemiBlock(uint256 blockHash)
{
    // Fetch block data from Demi-nodes
    DemiFetchBlock(blockHash);

    // Make sure we found a Demi-node
    if(!fDemiFound) {
        LogPrintf("Demi-node System: WARNING - No Demi-nodes found!\n");
        return false;
    }

    // Report success if we reach this point
    return true;
}

void DemiDataRefresh()
{
    // Ensure nDemiFetchIntervalNet is initialized
    if(nDemiFetchIntervalNet == NULL) {
        // Since first run, ask network for data
        DemiFetchLatest();

    }
    // Check if periodic data refresh is needed (default: 5 minutes)
    if(nDemiFetchIntervalNet + (5 * 60) < (GetTime())) {
        // Request updated data from network
        DemiFetchLatest();
    }
}

void DemiProcessRefresh()
{
    // Processes data from DemiDataRefresh()
}

void DemiConsensusLogix()
{
    // Takes processed data and merges to consensus network based on Demi-node data
}

bool getDemiNetStatus(int nTotalDemi, int nCurrentDemi)
{
    // Report success if we reach this point
    return true;
}

bool getDemiNodeInfo(std::vector<std::string> sDemiPeers, std::vector<int64_t> nDemiHeights, std::vector<uint256> vecDemiHashes)
{
    // Report success if we reach this point
    return true;
}
