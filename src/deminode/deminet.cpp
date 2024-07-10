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
    // Loop through Demi-node list
    ReadDemiConfigFile(peerAddr);
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
        BOOST_FOREACH(CNode* pnode, vNodes) {
            // Skip obsolete nodes
            if(pnode->nVersion < DEMINODE_VERSION) {
                continue;
            }

            // TODO: See if we found a Demi-node

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
