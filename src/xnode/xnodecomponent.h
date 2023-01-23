// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Darkcoin developers
// Copyright (c) 2017-2023 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XNODE_H
#define XNODE_H

#include "primitives/uint256.h"
#include "core/sync.h"
#include "node/net.h"
#include "subcore/key.h"
#include "util/util.h"
#include "primitives/base58.h"
#include "core/main.h"
#include "subcore/script.h"

class uint256;

#define XNODE_NOT_PROCESSED               0 // initial state
#define XNODE_IS_CAPABLE                  1
#define XNODE_NOT_CAPABLE                 2
#define XNODE_STOPPED                     3
#define XNODE_INPUT_TOO_NEW               4
#define XNODE_PORT_NOT_OPEN               6
#define XNODE_PORT_OPEN                   7
#define XNODE_SYNC_IN_PROCESS             8
#define XNODE_REMOTELY_ENABLED            9

#define XNODE_MIN_CONFIRMATIONS           7
#define XNODE_MIN_DSEEP_SECONDS           (30*60)
#define XNODE_MIN_DSEE_SECONDS            (5*60)
#define XNODE_PING_SECONDS                (1*60)
#define XNODE_EXPIRATION_SECONDS          (65*60)
#define XNODE_REMOVAL_SECONDS             (70*60)

using namespace std;

class CXNode;

extern CCriticalSection cs_xnodes;
extern map<int64_t, uint256> mapCacheBlockHashes;

bool GetBlockHash(uint256& hash, int nBlockHeight);

//
// The XNode Class. For managing the xnodeengine process. It contains the input of the 5,000 ESP, signature to prove
// it's the one who own that ip address and code for calculating the payment election.
//
class CXNode
{
private:
    // critical section to protect the inner data structures
    mutable CCriticalSection cs;

public:
    enum state {
        XNODE_ENABLED = 1,
        XNODE_EXPIRED = 2,
        XNODE_VIN_SPENT = 3,
        XNODE_REMOVE = 4,
        XNODE_POS_ERROR = 5
    };

    CTxIn vin;  
    CService addr;
    CPubKey pubkey;
    CPubKey pubkey2;
    std::vector<unsigned char> sig;
    int activeState;
    int64_t sigTime; //dsee message times
    int64_t lastDseep;
    int64_t lastTimeSeen;
    int cacheInputAge;
    int cacheInputAgeBlock;
    bool unitTest;
    bool allowFreeTx;
    int protocolVersion;
    int64_t nLastDsq; //the dsq count from the last dsq broadcast of this node
    CScript donationAddress;
    int donationPercentage;
    int nVote;
    int64_t lastVote;
    int nScanningErrorCount;
    int nLastScanningErrorBlockHeight;
    int64_t nLastPaid;


    CXNode();
    CXNode(const CXNode& other);
    CXNode(CService newAddr, CTxIn newVin, CPubKey newPubkey, std::vector<unsigned char> newSig, int64_t newSigTime, CPubKey newPubkey2, int protocolVersionIn, CScript donationAddress, int donationPercentage);


    void swap(CXNode& first, CXNode& second) // nothrow
    {
        // enable ADL (not necessary in our case, but good practice)
        using std::swap;

        // by swapping the members of two classes,
        // the two classes are effectively swapped
        swap(first.vin, second.vin);
        swap(first.addr, second.addr);
        swap(first.pubkey, second.pubkey);
        swap(first.pubkey2, second.pubkey2);
        swap(first.sig, second.sig);
        swap(first.activeState, second.activeState);
        swap(first.sigTime, second.sigTime);
        swap(first.lastDseep, second.lastDseep);
        swap(first.lastTimeSeen, second.lastTimeSeen);
        swap(first.cacheInputAge, second.cacheInputAge);
        swap(first.cacheInputAgeBlock, second.cacheInputAgeBlock);
        swap(first.unitTest, second.unitTest);
        swap(first.allowFreeTx, second.allowFreeTx);
        swap(first.protocolVersion, second.protocolVersion);
        swap(first.nLastDsq, second.nLastDsq);
        swap(first.donationAddress, second.donationAddress);
        swap(first.donationPercentage, second.donationPercentage);
        swap(first.nVote, second.nVote);
        swap(first.lastVote, second.lastVote);
        swap(first.nScanningErrorCount, second.nScanningErrorCount);
        swap(first.nLastScanningErrorBlockHeight, second.nLastScanningErrorBlockHeight);
        swap(first.nLastPaid, second.nLastPaid);
    }

    CXNode& operator=(CXNode from)
    {
        swap(*this, from);
        return *this;
    }
    friend bool operator==(const CXNode& a, const CXNode& b)
    {
        return a.vin == b.vin;
    }
    friend bool operator!=(const CXNode& a, const CXNode& b)
    {
        return !(a.vin == b.vin);
    }

    uint256 CalculateScore(int mod=1, int64_t nBlockHeight=0);

    IMPLEMENT_SERIALIZE
    (
        // serialized format:
        // * version byte (currently 0)
        // * all fields (?)
        {
                LOCK(cs);
                unsigned char nVersion = 0;
                READWRITE(nVersion);
                READWRITE(vin);
                READWRITE(addr);
                READWRITE(pubkey);
                READWRITE(pubkey2);
                READWRITE(sig);
                READWRITE(activeState);
                READWRITE(sigTime);
                READWRITE(lastDseep);
                READWRITE(lastTimeSeen);
                READWRITE(cacheInputAge);
                READWRITE(cacheInputAgeBlock);
                READWRITE(unitTest);
                READWRITE(allowFreeTx);
                READWRITE(protocolVersion);
                READWRITE(nLastDsq);
                READWRITE(donationAddress);
                READWRITE(donationPercentage);
                READWRITE(nVote);
                READWRITE(lastVote);
                READWRITE(nScanningErrorCount);
                READWRITE(nLastScanningErrorBlockHeight);
                READWRITE(nLastPaid);
        }
    )

    int64_t SecondsSincePayment()
    {
        return (GetAdjustedTime() - nLastPaid);
    }

    void UpdateLastSeen(int64_t override=0)
    {
        if(override == 0){
            lastTimeSeen = GetAdjustedTime();
        } else {
            lastTimeSeen = override;
        }
    }

    //TODO: Check and remove unused functions
    inline uint64_t SliceHash(uint256& hash, int slice)
    {
        uint64_t n = 0;
        memcpy(&n, &hash+slice*64, 64);
        return n;
    }

    void Check();

    bool UpdatedWithin(int seconds)
    {
        // LogPrintf("UpdatedWithin %d, %d --  %d \n", GetAdjustedTime() , lastTimeSeen, (GetAdjustedTime() - lastTimeSeen) < seconds);

        return (GetAdjustedTime() - lastTimeSeen) < seconds;
    }

    void Disable()
    {
        lastTimeSeen = 0;
    }

    bool IsEnabled()
    {
        return activeState == XNODE_ENABLED;
    }

    int GetXNodeInputAge()
    {
        if(pindexBest == NULL) return 0;

        if(cacheInputAge == 0){
            cacheInputAge = GetInputAge(vin);
            cacheInputAgeBlock = pindexBest->nHeight;
        }

        return cacheInputAge+(pindexBest->nHeight-cacheInputAgeBlock);
    }

    std::string Status() {
        std::string strStatus = "ACTIVE";

        if(activeState == CXNode::XNODE_ENABLED) strStatus   = "ENABLED";
        if(activeState == CXNode::XNODE_EXPIRED) strStatus   = "EXPIRED";
        if(activeState == CXNode::XNODE_VIN_SPENT) strStatus = "VIN_SPENT";
        if(activeState == CXNode::XNODE_REMOVE) strStatus    = "REMOVE";
        if(activeState == CXNode::XNODE_POS_ERROR) strStatus = "POS_ERROR";

        return strStatus;
    }
};

#endif
