// Copyright (c) 2009-2012 The Darkcoin developers
// Copyright (c) 2017-2018 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "key.h"
#include "util.h"
#include "script.h"
#include "base58.h"
#include "protocol.h"
#include "fork.h"
#include "main.h"
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

class CSporkMessage;
class CSporkManager;

CSporkManager sporkManager;

std::map<uint256, CSporkMessage> mapSporks;
std::map<int, CSporkMessage> mapSporksActive;

void ProcessSpork(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{
    if (strCommand == "spork")
    {
        //LogPrintf("ProcessSpork::spork\n");
        CDataStream vMsg(vRecv);
        CSporkMessage spork;
        vRecv >> spork;

        if(pindexBest == NULL) return;

        uint256 hash = spork.GetHash();
        if(mapSporksActive.count(spork.nSporkID)) {
            if(mapSporksActive[spork.nSporkID].nTimeSigned >= spork.nTimeSigned){
                if(fDebug) LogPrintf("spork - seen %s block %d \n", hash.ToString().c_str(), pindexBest->nHeight);
                return;
            } else {
                if(fDebug) LogPrintf("spork - got updated spork %s block %d \n", hash.ToString().c_str(), pindexBest->nHeight);
            }
        }

        LogPrintf("spork - new %s ID %d Time %d bestHeight %d\n", hash.ToString().c_str(), spork.nSporkID, spork.nValue, pindexBest->nHeight);

        if(!sporkManager.CheckSignature(spork)){
            LogPrintf("spork - invalid signature\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        mapSporks[hash] = spork;
        mapSporksActive[spork.nSporkID] = spork;
        sporkManager.Relay(spork);

        //does a task if needed
        ExecuteSpork(spork.nSporkID, spork.nValue);
    }
    if (strCommand == "getsporks")
    {
        std::map<int, CSporkMessage>::iterator it = mapSporksActive.begin();

        while(it != mapSporksActive.end()) {
            pfrom->PushMessage("spork", it->second);
            it++;
        }
    }

}

// grab the spork, otherwise say it's off
bool IsSporkActive(int nSporkID)
{
    int64_t r = -1;

    if(mapSporksActive.count(nSporkID)){
        r = mapSporksActive[nSporkID].nValue;
    } else {
        if(nSporkID == SPORK_1_XNODE_PAYMENTS_ENFORCEMENT) r = SPORK_1_XNODE_PAYMENTS_ENFORCEMENT_DEFAULT;
        if(nSporkID == SPORK_6_REPLAY_BLOCKS) r = SPORK_6_REPLAY_BLOCKS_DEFAULT;
        if(nSporkID == SPORK_7_XNODE_SCANNING) r = SPORK_7_XNODE_SCANNING;
        if(nSporkID == SPORK_8_XNODE_PAYMENT_ENFORCEMENT) r = SPORK_8_XNODE_PAYMENT_ENFORCEMENT_DEFAULT;
        if(nSporkID == SPORK_10_XNODE_PAY_UPDATED_NODES) r = SPORK_10_XNODE_PAY_UPDATED_NODES_DEFAULT;
        if(nSporkID == SPORK_12_RECONSIDER_BLOCKS) r = SPORK_12_RECONSIDER_BLOCKS_DEFAULT;

        if(r == -1) LogPrintf("GetSpork::Unknown Spork %d\n", nSporkID);
    }
    if(r == -1) r = 4070908800; //return 2099-1-1 by default

    return r < GetTime();
}

// grab the value of the spork on the network, or the default
int64_t GetSporkValue(int nSporkID)
{
    int64_t r = -1;

    if(mapSporksActive.count(nSporkID)){
        r = mapSporksActive[nSporkID].nValue;
    } else {
        if(nSporkID == SPORK_1_XNODE_PAYMENTS_ENFORCEMENT) r = SPORK_1_XNODE_PAYMENTS_ENFORCEMENT_DEFAULT;
        if(nSporkID == SPORK_6_REPLAY_BLOCKS) r = SPORK_6_REPLAY_BLOCKS_DEFAULT;
        if(nSporkID == SPORK_7_XNODE_SCANNING) r = SPORK_7_XNODE_SCANNING;
        if(nSporkID == SPORK_8_XNODE_PAYMENT_ENFORCEMENT) r = SPORK_8_XNODE_PAYMENT_ENFORCEMENT_DEFAULT;
        if(nSporkID == SPORK_10_XNODE_PAY_UPDATED_NODES) r = SPORK_10_XNODE_PAY_UPDATED_NODES_DEFAULT;
        if(nSporkID == SPORK_12_RECONSIDER_BLOCKS) r = SPORK_12_RECONSIDER_BLOCKS_DEFAULT;

        if(r == -1) LogPrintf("GetSpork::Unknown Spork %d\n", nSporkID);
    }

    return r;
}

void ReprocessBlocks(int nBlocks)
{
    // RepocessBlocks is not yet implemented
    LogPrintf("ReprocessBlocks -- Not yet implemented! Blocks = %d \n", nBlocks);
}

void ExecuteSpork(int nSporkID, int nValue)
{
    //correct fork via spork technology
    if(nSporkID == SPORK_12_RECONSIDER_BLOCKS && nValue > 0) {
        LogPrintf("Spork::ExecuteSpork -- Reconsider Last %d Blocks\n", nValue);

        ReprocessBlocks(nValue);
    }
}

bool CSporkManager::CheckSignature(CSporkMessage& spork)
{

    return true;
}

bool CSporkManager::Sign(CSporkMessage& spork)
{

    return true;
}

bool CSporkManager::UpdateSpork(int nSporkID, int64_t nValue)
{

    CSporkMessage msg;
    msg.nSporkID = nSporkID;
    msg.nValue = nValue;
    msg.nTimeSigned = GetTime();

    if(Sign(msg)){
        Relay(msg);
        mapSporks[msg.GetHash()] = msg;
        mapSporksActive[nSporkID] = msg;
        return true;
    }

    return false;
}

void CSporkManager::Relay(CSporkMessage& msg)
{
    CInv inv(MSG_SPORK, msg.GetHash());

    RelayInventory(inv);
}

bool CSporkManager::SetPrivKey(std::string strPrivKey)
{
    CSporkMessage msg;

    // Test signing successful, proceed
    strMasterPrivKey = strPrivKey;

    Sign(msg);

    if(CheckSignature(msg)){
        LogPrintf("CSporkManager::SetPrivKey - Successfully initialized as spork signer\n");
        return true;
    } else {
        return false;
    }
}

int CSporkManager::GetSporkIDByName(std::string strName)
{
    if(strName == "SPORK_1_XNODE_PAYMENTS_ENFORCEMENT") return SPORK_1_XNODE_PAYMENTS_ENFORCEMENT;
    if(strName == "SPORK_6_REPLAY_BLOCKS") return SPORK_6_REPLAY_BLOCKS;
    if(strName == "SPORK_7_XNODE_SCANNING") return SPORK_7_XNODE_SCANNING;
    if(strName == "SPORK_8_XNODE_PAYMENT_ENFORCEMENT") return SPORK_8_XNODE_PAYMENT_ENFORCEMENT;
    if(strName == "SPORK_10_XNODE_PAY_UPDATED_NODES") return SPORK_10_XNODE_PAY_UPDATED_NODES;
    if(strName == "SPORK_12_RECONSIDER_BLOCKS") return SPORK_12_RECONSIDER_BLOCKS;

    return -1;
}

std::string CSporkManager::GetSporkNameByID(int id)
{
    if(id == SPORK_1_XNODE_PAYMENTS_ENFORCEMENT) return "SPORK_1_XNODE_PAYMENTS_ENFORCEMENT";
    if(id == SPORK_6_REPLAY_BLOCKS) return "SPORK_6_REPLAY_BLOCKS";
    if(id == SPORK_7_XNODE_SCANNING) return "SPORK_7_XNODE_SCANNING";
    if(id == SPORK_8_XNODE_PAYMENT_ENFORCEMENT) return "SPORK_8_XNODE_PAYMENT_ENFORCEMENT";
    if(id == SPORK_10_XNODE_PAY_UPDATED_NODES) return "SPORK_10_XNODE_PAY_UPDATED_NODES";
    if(id == SPORK_12_RECONSIDER_BLOCKS) return "SPORK_12_RECONSIDER_BLOCKS";

    return "Unknown";
}
