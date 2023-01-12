// Copyright (c) 2022-2023 The CryptoCoderz Team / Espers project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PUBKEYALIASSERVICEMAN_H
#define PUBKEYALIASSERVICEMAN_H

#include "primitives/bignum.h"
#include "core/sync.h"
#include "node/net.h"
#include "subcore/key.h"
#include "core/chain.h"
#include "util/util.h"
#include "subcore/script.h"
#include "primitives/base58.h"
#include "core/main.h"

#include "pas.h"

#define PUBKEYALIASSERVICES_DUMP_SECONDS               (1*60)
#define PUBKEYALIASSERVICES_QUEUE_SECONDS              (1*30)

using namespace std;

class CPubkeyaliasserviceMan;

extern CPubkeyaliasserviceMan paserviceman;

void DumpPubkeyaliasservices();

/** Access to the PAS database (pascache.dat) */
class CPubkeyaliasserviceDB
{
private:
    boost::filesystem::path pathPAS;
    std::string strMagicMessage;
public:
    enum ReadResult {
        Ok,
        FileError,
        HashReadError,
        IncorrectHash,
        IncorrectMagicMessage,
        IncorrectMagicNumber,
        IncorrectFormat
    };

    CPubkeyaliasserviceDB();
    bool Write(const CPubkeyaliasserviceMan &paservicemanToSave);
    ReadResult Read(CPubkeyaliasserviceMan& paservicemanToLoad);
};

class CPubkeyaliasserviceMan
{
private:
    // critical section to protect the inner data structures
    mutable CCriticalSection cs;

    // map to hold all PASs
    std::vector<CPubkeyaliasservice> vPubkeyaliasservices;
    // who's asked for the pubkeyaliasservice list and the last time
    std::map<CNetAddr, int64_t> mAskedUsForPubkeyaliasserviceList;
    // who we asked for the pubkeyaliasservice list and the last time
    std::map<CNetAddr, int64_t> mWeAskedForPubkeyaliasserviceList;
    // which pubkeyaliasservices we've asked for
    std::map<COutPoint, int64_t> mWeAskedForPubkeyaliasserviceListEntry;

public:

    IMPLEMENT_SERIALIZE
    (
        // serialized format:
        // * version byte (currently 0)
        // * pubkeyaliasservices vector
        {
                LOCK(cs);
                unsigned char nVersion = 0;
                READWRITE(nVersion);
                READWRITE(vPubkeyaliasservices);
                READWRITE(mAskedUsForPubkeyaliasserviceList);
                READWRITE(mWeAskedForPubkeyaliasserviceList);
                READWRITE(mWeAskedForPubkeyaliasserviceListEntry);
        }
    )

    CPubkeyaliasserviceMan();
    CPubkeyaliasserviceMan(CPubkeyaliasserviceMan& other);

    // Add an entry
    bool AddPAS(CPubkeyaliasservice &pas);

    // Check all pubkeyaliasservices
    void CheckPAS();

    /// Ask (source) node for pasb
    void AskForPAS(CNode *pnode, CTxIn &vin);

    // Check all pubkeyaliasservices and remove inactive
    void CheckAndRemovePAS();

    // Clear pubkeyaliasservice vector
    void ClearPAS();

    // Remove pubkeyaliasservice entry
    void RemovePAS(CTxIn vin);

    // Find total PAS entries
    int CountEnabledPAS();

    // Find total PAS entries (Above protocol version N)
    int CountPubkeyaliasservicesAboveProtocolPAS(int minprotocolVersion);

    // Find an entry
    CPubkeyaliasservice* Find(const CTxIn& vin);
    CPubkeyaliasservice* Find(const CPubKey& pubKeyPubkeyaliasservice);

    //Find an entry that does not match every entry provided vector
    CPubkeyaliasservice* FindOldestNotInVec(const std::vector<CTxIn> &vVins, int nMinimumAge);

    std::vector<CPubkeyaliasservice> GetFullPubkeyaliasserviceVector() { CheckPAS(); return vPubkeyaliasservices; }

    void ProcessMessagePAS(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);

    // Return the number of (unique) pubkeyaliasservices
    int size() { return vPubkeyaliasservices.size(); }

    std::string ToString() const;

    //
    // Relay Pubkeyaliasservice Messages
    //

    void RelayPubkeyaliasserviceEntry(const CTxIn vin, const int64_t nregTime, const CPubKey pubkey, const int count, const int protocolVersion);
    void RelayPubkeyaliasserviceEntryPing(const CTxIn vin, const int64_t nregTime, const bool stop);

};

#endif
