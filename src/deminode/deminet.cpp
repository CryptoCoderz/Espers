// Copyright (c) 2021-2024 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "deminet.h"
#include "demimodule.h"

// Setup Demi network voting log
int voteDeminodes[4] {
    0, 0, 0, 0
};
// Setup stored Demi node heights/hashes
int heightDeminodes[4] {
    0, 0, 0, 0
};
uint256 hashDeminodes[4] {
    0, 0, 0, 0
};

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

static void DemiNodeFetch(uint256 blockHash)
{
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
                LogPrintf("Demi-node System: DemiNodeFetch - Requesting block %s from %s\n", blockHash.ToString().c_str(), pnode->addrName.c_str());
                pnode->PushMessage("getdata", dinv);
            }

        }

        vGetDemiData.clear();
        if(fDemiFound) {
            LogPrintf("Demi-node System: DemiNodeFetch - Requested data successfully!\n");
        }
}

bool getDemiBlock(uint256 blockHash)
{
    // Fetch block data from Demi-nodes
    DemiNodeFetch(blockHash);

    // Make sure we found a Demi-node
    if(!fDemiFound) {
        LogPrintf("Demi-node System: WARNING - No Demi-nodes found!\n");
        return false;
    }

    // Report success if we reach this point
    return true;
}
