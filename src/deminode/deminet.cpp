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
// First run toggle
bool fDemiInitializeFetch = false;

bool fDemiPeerRelay(std::string peerAddr)
{
    // Search through Registered Demi-node list
    FindRegisteredDemi(peerAddr);
    if(fDemiFound) {
        // Return success
        LogPrintf("Demi-node System: fDemiPeerRelay() - Peer: %s matches listed Demi-node!\n", peerAddr);
        return true;
    }
    return false;
}

void DemiDataRequestHandler(std::string peerAddr, bool fDemiFetched, bool fDemiSingleShot, const CInv dinv) {
    // Node data Lock
    LOCK(cs_vNodes);
    // Loop through peers/nodes
    for(CNode* pnode : vNodes) {
        // Skip obsolete nodes
        if(pnode->nVersion < DEMINODE_VERSION) {
            continue;
        }
        // Request block data from Demi-node(s)
        if(fDemiSingleShot) {
            if(pnode->addrName == peerAddr) {
                pnode->PushMessage("getdata", dinv);
                fDemiFetched = true;
                break;
            }
        } else if(fDemiPeerRelay(pnode->addrName)) {
            pnode->PushMessage("getdata", dinv);
            if(!fDemiFetched) {
                fDemiFetched = true;
            }
        }
    }
}

void DemiFetchBlock(uint256 blockHash)
{
    // Setup required values
    std::string peerAddr = "127.0.0.1";
    // block allocation
    const CInv& dinv = CInv(MSG_DEMIBLOCK, blockHash);
    // Reset previously set data
    fDemiFound = false;
    // Run Demi-node data request handler
    DemiDataRequestHandler(peerAddr, false, false, dinv);
    // Update last data fetch time
    nDemiFetchIntervalNet = GetTime();

    if(fDemiFound) {
        LogPrintf("Demi-node System: DemiFetchBlock() - Requested specific block data successfully!\n");
    }
}

void DemiFetchLatest(bool fDemiFetched)
{
    // Setup required values
    std::string peerAddr = "127.0.0.1";
    uint256 blockHash = Params().GenesisBlock().GetHash();
    // block allocation
    const CInv& dinv = CInv(MSG_DEMICURRENT, blockHash);
    // Run Demi-node data request handler
    DemiDataRequestHandler(peerAddr, fDemiFetched, false, dinv);
    // Update last data fetch time
    nDemiFetchIntervalNet = GetTime();

    if(fDemiFetched) {
        LogPrintf("Demi-node System: DemiFetchLatest() - Requested Latest Block data successfully!\n");
    }
}

void DemiFetchMulti(uint256 blockHash, std::string peerAddr, bool fDemiFetched)
{
    // block allocation
    const CInv& dinv = CInv(MSG_DEMIMULTI, blockHash);
    // Run Demi-node data request handler
    DemiDataRequestHandler(peerAddr, fDemiFetched, true, dinv);

    if(fDemiFetched) {
        LogPrintf("Demi-node System: DemiFetchMulti() - Requested Block data chunk from Demi-node\n");
    }
}

bool getDemiBlock(uint256 blockHash)
{
    // Fetch block data from Demi-nodes
    DemiFetchBlock(blockHash);

    // Make sure we found a Demi-node
    if(!fDemiFound) {
        LogPrintf("Demi-node System: getDemiBlock() - Could not fetch any data, no Demi-nodes found!\n");
        return false;
    }

    // Report success if we reach this point
    return true;
}

void DemiDataRefresh()
{
    bool fDemiFetched = false;

    // Ensure nDemiFetchIntervalNet is initialized
    if(nDemiFetchIntervalNet == NULL) {
        // Initialize data fetch time
        nDemiFetchIntervalNet = GetTime();
        fDemiInitializeFetch = true;
    }
    // Check if periodic data refresh is needed (default: 5 minutes)
    if(nDemiFetchIntervalNet + (5 * 60) < (GetTime())) {
        // Request updated data from network
        DemiFetchLatest(fDemiFetched);
    } else if(fDemiInitializeFetch) {
        // Shorter wait for initial run/startup
        if(nDemiFetchIntervalNet + (1 * 60) < (GetTime())) {
            // Request updated data from network
            DemiFetchLatest(fDemiFetched);
            fDemiInitializeFetch = false;
        }
    }

    // Make sure we found a Demi-node
    if(!fDemiFetched) {
        LogPrintf("Demi-node System: DemiDataRefresh() - Could not fetch any data, no Demi-nodes found!\n");
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
