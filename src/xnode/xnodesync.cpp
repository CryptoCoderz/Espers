// Copyright (c) 2014-2016 The Dash developers
// Copyright (c) 2017-2024 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xnodesync.h"
#include "xnodemngr.h"

class CXNodeSync;
CXNodeSync xnodeSync;

bool CXNodeSync::IsBlockchainSynced()
{
    static bool fBlockchainSynced = false;
    static int64_t lastProcess = GetTime();
    nAssetSyncStarted = GetTime();

    // if the last call to this function was more than 60 minutes ago (client was in sleep mode) reset the sync process
    if(GetTime() - lastProcess > 60*60) {
        lastXNsyncFailure = GetTime();
        RequestedXNodeSyncAttempt++;
        fBlockchainSynced = false;
        warningXNodeSync++;
        LogPrintf("CXNodeSync::IsXNodeSynced - WARNING - Client was in sleep mode, checking sync status\n");
        return false;
    }
    lastProcess = GetTime();

    if(fBlockchainSynced) return true;

    if (fImporting || fReindex) return false;

    TRY_LOCK(cs_main, lockMain);
    if(!lockMain) return false;

    CBlockIndex* pindex = pindexBest;
    if(pindex == NULL) return false;


    if(pindex->nTime + 60*60 < GetTime())
        return false;

    fBlockchainSynced = true;

    return true;
}

bool CXNodeSync::IsXNodeSynced()
{
    // Verify mainnet sync status
    if(IsBlockchainSynced()) {
        // Resync if we lose all xnodes from sleep/wake or failure to sync originally
        // Keep looping for a while but count the attempts
        //LogPrintf("CXNodeSync::IsXNodeSynced - INFO - Mainnet sync'd, running other checks\n");
        if(xnodeman.CountEnabled() == 0) {
            lastXNsyncFailure = GetTime();
            RequestedXNodeSyncAttempt += 2;
            //LogPrintf("CXNodeSync::IsXNodeSynced - WARNING - No XNodes on sync'd Mainnet\n");
        }
    } else if(!IsBlockchainSynced()) {
        // Keep looping for a while but count the attempts
        lastXNsyncFailure = GetTime();
        RequestedXNodeSyncAttempt += 2;
        //LogPrintf("CXNodeSync::IsXNodeSynced - INFO - Mainnet not sync'd, timeout and wait\n");
    }

    // Check for timeout
    if(RequestedXNodeSyncAttempt >= XNODE_SYNC_THRESHOLD || GetTime() - nAssetSyncStarted > XNODE_SYNC_TIMEOUT)
    {
        // Report failed sync
        // TODO: add more extensive and stringent security checks
        RequestedXNodeSyncAttempt = 0;
        warningXNodeSync = 0;
        lastXNsyncFailure = GetTime();
        //LogPrintf("CXNodeSync::IsXNodeSynced - ERROR - Sync has failed, retry again later\n");
        MilliSleep(30000);// TODO: Find alternative to sleeping, maybe per-block bool trigger?
        return false;
    }

    // XNode reward enforcement (TODO: finish implementation)
    if(IsSporkActive(SPORK_8_XNODE_PAYMENT_ENFORCEMENT)) {
        warningXNodeSync++;
        //LogPrintf("CXNodeSync::IsXNodeSynced - WARNING - Reward enforcement not yet implemented\n");
    }



    // Report successfull sync
    // TODO: add more extensive and stringent security checks
    if(warningXNodeSync >= XNODE_SYNC_WARNING) {
        //LogPrintf("CXNodeSync::IsXNodeSynced - SUCCESS - Sync finished but with WARNINGS\n");
        warningXNodeSync = 0;
        return true;
    } else {
        // TODO: Rework this function properly, for now disable pointless debug warnings
        //LogPrintf("CXNodeSync::IsXNodeSynced - SUCCESS - Sync has finished\n");
        return true;
    }

}
