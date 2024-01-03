// Copyright (c) 2022-2024 The CryptoCoderz Team / Espers project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pasengine.h"
#include "core/main.h"
#include "util/init.h"
#include "util/util.h"
#include "pasman.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <boost/assign/list_of.hpp>
#include <openssl/rand.h>

using namespace std;
using namespace boost;

// The main object for accessing pasengine
CPASenginePool pasEnginePool;
// A helper object for associating messages from Pubkeyaliasservice
CPASengineAssociator pasEngineAssociator;

bool CPASenginePool::IsBlockchainSynced()
{
    static bool fBlockchainSynced = false;
    static int64_t lastProcess = GetTime();

    // if the last call to this function was more than 60 minutes ago (client was in sleep mode) reset the sync process
    if(GetTime() - lastProcess > 60*60) {
        fBlockchainSynced = false;
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

// TODO: ensure we compare Vin key against Vout destination key
bool CPASengineAssociator::IsVinAssociatedWithPubkey(CTxIn& vin, CPubKey& pubkey){
    CScript payee2;
    payee2 = GetScriptForDestination(pubkey.GetID());

    CTransaction txVin;
    uint256 hash;

    if(GetTransaction(vin.prevout.hash, txVin, hash)){
        BOOST_FOREACH(CTxOut out, txVin.vout){
            if(out.nValue == PubkeyaliasserviceFEE(pindexBest->nHeight)*COIN){
                if(out.scriptPubKey == payee2) return true;
            }
        }
    }

    return false;
}

// TODO: ensure we compare Vout key against PAS destination key
bool CPASengineAssociator::IsVoutAssociatedWithPubkey(CTxIn& vin, CPubKey& PASpubkey){
    CScript PASkey;
    PASkey = GetScriptForDestination(PASpubkey.GetID());

    CTransaction txVin;
    uint256 hash;

    if(GetTransaction(vin.prevout.hash, txVin, hash)){
        BOOST_FOREACH(CTxOut out, txVin.vout){
            if(out.nValue == PubkeyaliasserviceFEE(pindexBest->nHeight)*COIN){
                if(vin.prevPubKey == PASkey) return true;
            }
        }
    }

    return false;
}
