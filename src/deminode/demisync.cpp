// Copyright (c) 2021-2025 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "demisync.h"
#include "deminet.h"
#include "demimodule.h"

static void DemiNodeSelect(bool demiSelectStatus, std::string demiSelectAddr)
{
        LOCK(cs_vNodes);
        // Reset previously set data
        fDemiFound = false;

        // Loop through peers/nodes
        for(CNode* pnode : vNodes) {
            // Skip obsolete nodes
            if(pnode->nVersion < DEMINODE_VERSION) {
                continue;
            }

            // See if we found a Demi-node
            if(fDemiPeerRelay(pnode->addrName)) {
                LogPrintf("Demi-node System: DemiNodeSelect - Selected a Demi-node successfully!\n");
                demiSelectStatus = true;
                demiSelectAddr = pnode->addrName;
                // Toggle sync status and triggers
                pnodeSync = pnode;
                PushGetBlocks(pnode, pindexBest, uint256(0));
                break;
            }
        }

        // Return silently (success)
        if(demiSelectStatus) {
            return;
        }

        // Report failure
        // LogPrintf("Demi-node System: DemiNodeSelect - Could not select a Demi-node\n");
}

bool startDemiSync()
{
    // Set values
    bool demiSelectStatus = false;
    std::string demiSelectAddr = "";
    // Fetch block data from Demi-nodes
    DemiNodeSelect(demiSelectStatus, demiSelectAddr);

    // Make sure we found a Demi-node
    if(!demiSelectStatus) {
        // LogPrintf("Demi-node System: WARNING - No connected Demi-nodes found for Demi-sync!\n");
        return false;
    } else {
        LogPrintf("Demi-node System: STATUS - Demi-sync has connected to %s and is downloading blocks!\n", demiSelectAddr.c_str());
        // Report success if we reach this point
        return true;
    }
}
