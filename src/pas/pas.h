// Copyright (c) 2022-2023 The CryptoCoderz Team / Espers project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PUBKEYALIASSERVICE_H
#define PUBKEYALIASSERVICE_H

#include "primitives/uint256.h"
#include "core/sync.h"
#include "node/net.h"
#include "subcore/key.h"
#include "util/util.h"
#include "primitives/base58.h"
#include "core/main.h"
#include "subcore/script.h"

class uint256;

#define PUBKEYALIASSERVICE_NOT_PROCESSED               0 // initial state
#define PUBKEYALIASSERVICE_IS_CAPABLE                  1
#define PUBKEYALIASSERVICE_NOT_CAPABLE                 2
#define PUBKEYALIASSERVICE_STOPPED                     3
#define PUBKEYALIASSERVICE_INPUT_TOO_NEW               4
#define PUBKEYALIASSERVICE_SYNC_IN_PROCESS             8

#define PUBKEYALIASSERVICE_MIN_CONFIRMATIONS           7
#define PUBKEYALIASSERVICE_EXPIRATION_SECONDS          (31600000)// 365 Days
#define PUBKEYALIASSERVICE_REMOVAL_SECONDS             (7*60)

using namespace std;

class CPubkeyaliasservice;

extern CCriticalSection cs_pubkeyaliasservices;
extern map<int64_t, uint256> mapCacheBlockHashesPAS;

bool GetBlockHashPAS(uint256& hash, int nBlockHeight);

//
// The Pubkeyaliasservice Class. For managing the pasengine process. It contains the input of the 78 FROG, signature to prove
// it's the one who owns that pubkey address.
//
class CPubkeyaliasservice
{
private:
    // critical section to protect the inner data structures
    mutable CCriticalSection cs;

public:
    enum state {
        PUBKEYALIASSERVICE_ENABLED = 1,
        PUBKEYALIASSERVICE_EXPIRED = 2,
        PUBKEYALIASSERVICE_VIN_ERROR = 3,
        PUBKEYALIASSERVICE_REMOVE = 4,
        PUBKEYALIASSERVICE_POS_ERROR = 5
    };

    CTxIn vin;
    CPubKey pubkey;
    int activeStatePAS;
    int cacheInputAge;
    int cacheInputAgeBlock;
    bool ioTestPAS;
    int protocolVersion;
    int nScanningErrorCount;
    int nLastScanningErrorBlockHeight;
    int64_t regTime;

    CPubkeyaliasservice();
    CPubkeyaliasservice(const CPubkeyaliasservice& other);
    CPubkeyaliasservice(CTxIn newVin, CPubKey newPubkey, int64_t newRegTime, int protocolVersionIn);


    void swap(CPubkeyaliasservice& first, CPubkeyaliasservice& second) // nothrow
    {
        // enable ADL (not necessary in our case, but good practice)
        using std::swap;

        // by swapping the members of two classes,
        // the two classes are effectively swapped
        swap(first.vin, second.vin);
        swap(first.pubkey, second.pubkey);
        swap(first.regTime, second.regTime);
        swap(first.cacheInputAge, second.cacheInputAge);
        swap(first.cacheInputAgeBlock, second.cacheInputAgeBlock);
        swap(first.ioTestPAS, second.ioTestPAS);
        swap(first.protocolVersion, second.protocolVersion);
        swap(first.nScanningErrorCount, second.nScanningErrorCount);
        swap(first.nLastScanningErrorBlockHeight, second.nLastScanningErrorBlockHeight);
    }

    CPubkeyaliasservice& operator=(CPubkeyaliasservice from)
    {
        swap(*this, from);
        return *this;
    }
    friend bool operator==(const CPubkeyaliasservice& a, const CPubkeyaliasservice& b)
    {
        return a.vin == b.vin;
    }
    friend bool operator!=(const CPubkeyaliasservice& a, const CPubkeyaliasservice& b)
    {
        return !(a.vin == b.vin);
    }


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
                READWRITE(pubkey);
                READWRITE(regTime);
                READWRITE(activeStatePAS);
                READWRITE(cacheInputAge);
                READWRITE(cacheInputAgeBlock);
                READWRITE(ioTestPAS);
                READWRITE(protocolVersion);
                READWRITE(nScanningErrorCount);
                READWRITE(nLastScanningErrorBlockHeight);
        }
    )
    
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
        return true;
        //return (GetAdjustedTime() - lastTimeSeen) < seconds;
    }

    void Disable()
    {
        //lastTimeSeen = 0;
    }

    bool IsEnabled()
    {
        return activeStatePAS == PUBKEYALIASSERVICE_ENABLED;
    }

    int GetPubkeyaliasserviceInputAge()
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

        if(activeStatePAS == CPubkeyaliasservice::PUBKEYALIASSERVICE_ENABLED) strStatus   = "ENABLED";
        if(activeStatePAS == CPubkeyaliasservice::PUBKEYALIASSERVICE_EXPIRED) strStatus   = "EXPIRED";
        if(activeStatePAS == CPubkeyaliasservice::PUBKEYALIASSERVICE_VIN_ERROR) strStatus = "VIN_ERROR";
        if(activeStatePAS == CPubkeyaliasservice::PUBKEYALIASSERVICE_REMOVE) strStatus    = "REMOVE";
        if(activeStatePAS == CPubkeyaliasservice::PUBKEYALIASSERVICE_POS_ERROR) strStatus = "POS_ERROR";

        return strStatus;
    }
};

#endif
