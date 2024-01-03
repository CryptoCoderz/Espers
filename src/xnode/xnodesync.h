// Copyright (c) 2014-2016 The Dash developers
// Copyright (c) 2017-2024 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XNODE_SYNC_H
#define XNODE_SYNC_H

#include "xnodereward.h"

static const int32_t XNODE_SYNC_TIMEOUT    = 25;
static const int32_t XNODE_SYNC_THRESHOLD  = 2;
static const int32_t XNODE_SYNC_WARNING    = 1;

class CXNodeSync;
extern CXNodeSync xnodeSync;

//
// CXNodeSync : check for logical xnode sync (TODO: implement in depth checks)
//

class CXNodeSync
{
public:
bool IsBlockchainSynced();
bool IsXNodeSynced();
int64_t warningXNodeSync;
int64_t RequestedXNodeSyncAttempt;
int64_t lastXNsyncFailure;
int64_t nAssetSyncStarted;
};

#endif // XNODE_SYNC_H
