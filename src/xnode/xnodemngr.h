// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2017-2025 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XNODEMAN_H
#define XNODEMAN_H

#include "primitives/bignum.h"
#include "core/sync.h"
#include "node/net.h"
#include "subcore/key.h"
#include "core/chain.h"
#include "util/util.h"
#include "subcore/script.h"
#include "primitives/base58.h"
#include "core/main.h"
#include "xnodecomponent.h"

#define XNODES_DUMP_SECONDS               (15*60)
#define XNODES_DSEG_SECONDS               (3*60*60)

using namespace std;

class CXNodeMan;

extern CXNodeMan xnodeman;

void DumpXNodes();

/** Access to the XN database (xnodelist.dat) */
class CXNodeDB
{
private:
    boost::filesystem::path pathXN;
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

    CXNodeDB();
    bool Write(const CXNodeMan &xnodemanToSave);
    ReadResult Read(CXNodeMan& xnodemanToLoad);
};

class CXNodeMan
{
private:
    // critical section to protect the inner data structures
    mutable CCriticalSection cs;

    // map to hold all XNs
    std::vector<CXNode> vXNodes;
    // who's asked for the xnode list and the last time
    std::map<CNetAddr, int64_t> mAskedUsForXNodeList;
    // who we asked for the xnode list and the last time
    std::map<CNetAddr, int64_t> mWeAskedForXNodeList;
    // which xnodes we've asked for
    std::map<COutPoint, int64_t> mWeAskedForXNodeListEntry;

public:
    // keep track of dsq count to prevent xnodes from gaming xnodeengine queue
    int64_t nDsqCount;

    IMPLEMENT_SERIALIZE
    (
        // serialized format:
        // * version byte (currently 0)
        // * xnodes vector
        {
                LOCK(cs);
                unsigned char nVersion = 0;
                READWRITE(nVersion);
                READWRITE(vXNodes);
                READWRITE(mAskedUsForXNodeList);
                READWRITE(mWeAskedForXNodeList);
                READWRITE(mWeAskedForXNodeListEntry);
                READWRITE(nDsqCount);
        }
    )

    CXNodeMan();
    CXNodeMan(CXNodeMan& other);

    // Add an entry
    bool Add(CXNode &xn);

    // Check all xnodes
    void Check();

    /// Ask (source) node for xnb
    void AskForXN(CNode *pnode, CTxIn &vin);

    // Check all xnodes and remove inactive
    void CheckAndRemove();

    // Clear xnode vector
    void Clear();

    int CountEnabled(int protocolVersion = -1);

    int CountXNodesAboveProtocol(int protocolVersion);

    void DsegUpdate(CNode* pnode);

    // Find an entry
    CXNode* Find(const CTxIn& vin);
    CXNode* Find(const CPubKey& pubKeyXNode);

    //Find an entry thta do not match every entry provided vector
    CXNode* FindOldestNotInVec(const std::vector<CTxIn> &vVins, int nMinimumAge);

    // Find a random entry
    CXNode* FindRandom();

    /// Find a random entry
    CXNode* FindRandomNotInVec(std::vector<CTxIn> &vecToExclude, int protocolVersion = -1);

    // Get the current winner for this block
    CXNode* GetCurrentXnode(int mod=1, int64_t nBlockHeight=0, int minProtocol=0);

    std::vector<CXNode> GetFullXNodeVector() { Check(); return vXNodes; }

    std::vector<pair<int, CXNode> > GetXNodeRanks(int64_t nBlockHeight, int minProtocol=0);
    int GetXNodeRank(const CTxIn &vin, int64_t nBlockHeight, int minProtocol=0, bool fOnlyActive=true);
    CXNode* GetXNodeByRank(int nRank, int64_t nBlockHeight, int minProtocol=0, bool fOnlyActive=true);

    void ProcessXNodeConnections();

    void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);

    // Return the number of (unique) xnodes
    int size() { return vXNodes.size(); }

    std::string ToString() const;

    //
    // Relay XNode Messages
    //

    void RelayXNodeEntry(const CTxIn vin, const CService addr, const std::vector<unsigned char> vchSig, const int64_t nNow, const CPubKey pubkey, const CPubKey pubkey2, const int count, const int current, const int64_t lastUpdated, const int protocolVersion, CScript donationAddress, int donationPercentage);
    void RelayXNodeEntryPing(const CTxIn vin, const std::vector<unsigned char> vchSig, const int64_t nNow, const bool stop);

    void Remove(CTxIn vin);

};

#endif
