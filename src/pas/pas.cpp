// Copyright (c) 2022-2025 The CryptoCoderz Team / Espers project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pas.h"
#include "pasman.h"
#include "pasengine.h"
#include "core/chain.h"
#include "core/main.h"
#include "core/sync.h"
#include "util/util.h"

#include "util/init.h"// TODO: If crashes check this

#include <boost/lexical_cast.hpp>


CCriticalSection cs_pubkeyaliasservices;
// keep track of the scanning errors I've seen
map<uint256, int> mapSeenPubkeyAliasServiceScanningErrors;
// cache block hashes as we calculate them
std::map<int64_t, uint256> mapCacheBlockHashesPAS;

//Get the last hash that matches the modulus given. Processed in reverse order
bool GetBlockHashPAS(uint256& hash, int nBlockHeight)
{
    if (pindexBest == NULL) return false;

    if(nBlockHeight == 0)
        nBlockHeight = pindexBest->nHeight;

    if(mapCacheBlockHashesPAS.count(nBlockHeight)){
        hash = mapCacheBlockHashesPAS[nBlockHeight];
        return true;
    }

    const CBlockIndex *BlockLastSolved = pindexBest;
    const CBlockIndex *BlockReading = pindexBest;

    if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || pindexBest->nHeight+1 < nBlockHeight) return false;

    int nBlocksAgo = 0;
    if(nBlockHeight > 0) nBlocksAgo = (pindexBest->nHeight+1)-nBlockHeight;
    assert(nBlocksAgo >= 0);

    int n = 0;
    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if(n >= nBlocksAgo){
            hash = BlockReading->GetBlockHash();
            mapCacheBlockHashesPAS[nBlockHeight] = hash;
            return true;
        }
        n++;

        if (BlockReading->pprev == NULL) { assert(BlockReading); break; }
        BlockReading = BlockReading->pprev;
    }

    return false;
}

CPubkeyaliasservice::CPubkeyaliasservice()
{
    LOCK(cs);
    vin = CTxIn();
    pubkey = CPubKey();
    activeStatePAS = PUBKEYALIASSERVICE_ENABLED;
    regTime = GetAdjustedTime();
    cacheInputAge = 0;
    cacheInputAgeBlock = 0;
    ioTestPAS = false;
    protocolVersion = MIN_PEER_PROTO_VERSION;
    nScanningErrorCount = 0;
    nLastScanningErrorBlockHeight = 0;
}

CPubkeyaliasservice::CPubkeyaliasservice(const CPubkeyaliasservice& other)
{
    LOCK(cs);
    vin = other.vin;
    pubkey = other.pubkey;
    activeStatePAS = other.activeStatePAS;
    regTime = other.regTime;
    cacheInputAge = other.cacheInputAge;
    cacheInputAgeBlock = other.cacheInputAgeBlock;
    ioTestPAS = other.ioTestPAS;
    protocolVersion = other.protocolVersion;
    nScanningErrorCount = other.nScanningErrorCount;
    nLastScanningErrorBlockHeight = other.nLastScanningErrorBlockHeight;
}

CPubkeyaliasservice::CPubkeyaliasservice(CTxIn newVin, CPubKey newPubkey, int64_t newRegTime, int protocolVersionIn)
{
    LOCK(cs);
    vin = newVin;
    pubkey = newPubkey;
    activeStatePAS = PUBKEYALIASSERVICE_ENABLED;
    regTime = newRegTime;
    cacheInputAge = 0;
    cacheInputAgeBlock = 0;
    ioTestPAS = false;
    protocolVersion = protocolVersionIn;
    nScanningErrorCount = 0;
    nLastScanningErrorBlockHeight = 0;
}

void CPubkeyaliasservice::Check()
{
    if(ShutdownRequested()) return;

    //TODO: Random segfault with this line removed
    TRY_LOCK(cs_main, lockRecv);
    if(!lockRecv) return;

    //once spent, stop doing the checks
    if(activeStatePAS == PUBKEYALIASSERVICE_VIN_ERROR) return;


    if(!UpdatedWithin(PUBKEYALIASSERVICE_REMOVAL_SECONDS)){
        activeStatePAS = PUBKEYALIASSERVICE_REMOVE;
        return;
    }

    if((regTime + PUBKEYALIASSERVICE_EXPIRATION_SECONDS) < GetTime()) {
        activeStatePAS = PUBKEYALIASSERVICE_EXPIRED;
        return;
    }

    if(!ioTestPAS){// bool is always false, TODO: handle previously verified PAS entries better
        CValidationState state;
        CScript pas_addr;
        CTransaction txVin;
        uint256 hash;
        // verify address
        if(pubkey.IsValid()) {
            pas_addr = GetScriptForDestination(pubkey.GetID());
        } else {
            LogPrintf("CreateNewBlock(): Failed to detect PASfee address \n");
        }
        if(GetTransaction(vin.prevout.hash, txVin, hash)) {
            BOOST_FOREACH(CTxOut out, txVin.vout){
                if(out.nValue == PubkeyaliasserviceFEE(pindexBest->nHeight)*COIN){
                    if(vin.prevPubKey == pas_addr) {
                        activeStatePAS = PUBKEYALIASSERVICE_ENABLED;
                    }
                }
            }
        } else {
            activeStatePAS = PUBKEYALIASSERVICE_VIN_ERROR;
            return;
        }

    } else {
        activeStatePAS = PUBKEYALIASSERVICE_ENABLED; // OK
    }
}
