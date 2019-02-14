// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2017-2018 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xnodemngr.h"
#include "xnodecomponent.h"
#include "xnodestart.h"
#include "xnodecrypter.h"
#include "chain.h"
#include "util.h"
#include "addrman.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>


/** XNode manager */
CXNodeMan xnodeman;
CCriticalSection cs_process_message;

struct CompareValueOnly
{
    bool operator()(const pair<int64_t, CTxIn>& t1,
                    const pair<int64_t, CTxIn>& t2) const
    {
        return t1.first < t2.first;
    }
};
struct CompareValueOnlyXN
{
    bool operator()(const pair<int64_t, CXNode>& t1,
                    const pair<int64_t, CXNode>& t2) const
    {
        return t1.first < t2.first;
    }
};


//
// CXNodeDB
//

CXNodeDB::CXNodeDB()
{
    pathXN = GetDataDir() / "xnodelist.dat";
    strMagicMessage = "XNodeCache";
}

bool CXNodeDB::Write(const CXNodeMan& xnodemanToSave)
{
    int64_t nStart = GetTimeMillis();

    // serialize addresses, checksum data up to that point, then append csum
    CDataStream ssXNodes(SER_DISK, CLIENT_VERSION);
    ssXNodes << strMagicMessage; // xnode cache file specific magic message
    ssXNodes << FLATDATA(Params().MessageStart()); // network specific magic number
    ssXNodes << xnodemanToSave;
    uint256 hash = Hash(ssXNodes.begin(), ssXNodes.end());
    ssXNodes << hash;

    // open output file, and associate with CAutoFile
    FILE *file = fopen(pathXN.string().c_str(), "wb");
    CAutoFile fileout = CAutoFile(file, SER_DISK, CLIENT_VERSION);
    if (!fileout)
        return error("%s : Failed to open file %s", __func__, pathXN.string());

    // Write and commit header, data
    try {
        fileout << ssXNodes;
    }
    catch (std::exception &e) {
        return error("%s : Serialize or I/O error - %s", __func__, e.what());
    }
    FileCommit(fileout);
    fileout.fclose();

    LogPrintf("Written info to xnodelist.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrintf("  %s\n", xnodemanToSave.ToString());

    return true;
}

CXNodeDB::ReadResult CXNodeDB::Read(CXNodeMan& xnodemanToLoad)
{
    int64_t nStart = GetTimeMillis();
    // open input file, and associate with CAutoFile
    FILE *file = fopen(pathXN.string().c_str(), "rb");
    CAutoFile filein = CAutoFile(file, SER_DISK, CLIENT_VERSION);
    if (!filein)
    {
        error("%s : Failed to open file %s", __func__, pathXN.string());
        return FileError;
    }

    // use file size to size memory buffer
    int fileSize = boost::filesystem::file_size(pathXN);
    int dataSize = fileSize - sizeof(uint256);
    // Don't try to resize to a negative number if file is small
    if (dataSize < 0)
        dataSize = 0;
    vector<unsigned char> vchData;
    vchData.resize(dataSize);
    uint256 hashIn;

    // read data and checksum from file
    try {
        filein.read((char *)&vchData[0], dataSize);
        filein >> hashIn;
    }
    catch (std::exception &e) {
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return HashReadError;
    }
    filein.fclose();

    CDataStream ssXNodes(vchData, SER_DISK, CLIENT_VERSION);

    // verify stored checksum matches input data
    uint256 hashTmp = Hash(ssXNodes.begin(), ssXNodes.end());
    if (hashIn != hashTmp)
    {
        error("%s : Checksum mismatch, data corrupted", __func__);
        return IncorrectHash;
    }

    unsigned char pchMsgTmp[4];
    std::string strMagicMessageTmp;
    try {
        // de-serialize file header (xnode cache file specific magic message) and ..

        ssXNodes >> strMagicMessageTmp;

        // ... verify the message matches predefined one
        if (strMagicMessage != strMagicMessageTmp)
        {
            error("%s : Invalid xnode cache magic message", __func__);
            return IncorrectMagicMessage;
        }

        // de-serialize file header (network specific magic number) and ..
        ssXNodes >> FLATDATA(pchMsgTmp);

        // ... verify the network matches ours
        if (memcmp(pchMsgTmp, Params().MessageStart(), sizeof(pchMsgTmp)))
        {
            error("%s : Invalid network magic number", __func__);
            return IncorrectMagicNumber;
        }

        // de-serialize address data into one CXnList object
        ssXNodes >> xnodemanToLoad;
    }
    catch (std::exception &e) {
        xnodemanToLoad.Clear();
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return IncorrectFormat;
    }

    xnodemanToLoad.CheckAndRemove(); // clean out expired
    LogPrintf("Loaded info from xnodelist.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrintf("  %s\n", xnodemanToLoad.ToString());

    return Ok;
}

void DumpXNodes()
{
    int64_t nStart = GetTimeMillis();

    CXNodeDB xndb;
    CXNodeMan tempXnodeman;

    LogPrintf("Verifying xnodelist.dat format...\n");
    CXNodeDB::ReadResult readResult = xndb.Read(tempXnodeman);
    // there was an error and it was not an error on file openning => do not proceed
    if (readResult == CXNodeDB::FileError)
        LogPrintf("Missing xnode list file - xnodelist.dat, will try to recreate\n");
    else if (readResult != CXNodeDB::Ok)
    {
        LogPrintf("Error reading xnodelist.dat: ");
        if(readResult == CXNodeDB::IncorrectFormat)
            LogPrintf("magic is ok but data has invalid format, will try to recreate\n");
        else
        {
            LogPrintf("file format is unknown or invalid, please fix it manually\n");
            return;
        }
    }
    LogPrintf("Writting info to xnodelist.dat...\n");
    xndb.Write(xnodeman);

    LogPrintf("XNode dump finished  %dms\n", GetTimeMillis() - nStart);
}

CXNodeMan::CXNodeMan() {
    nDsqCount = 0;
}

bool CXNodeMan::Add(CXNode &xn)
{
    LOCK(cs);

    if (!xn.IsEnabled())
        return false;

    CXNode *pxn = Find(xn.vin);

    if (pxn == NULL)
    {
        LogPrint("xnode", "CXNodeMan: Adding new xnode %s - %i now\n", xn.addr.ToString().c_str(), size() + 1);
        vXNodes.push_back(xn);
        return true;
    }

    return false;
}

void CXNodeMan::AskForXN(CNode* pnode, CTxIn &vin)
{
    std::map<COutPoint, int64_t>::iterator i = mWeAskedForXNodeListEntry.find(vin.prevout);
    if (i != mWeAskedForXNodeListEntry.end())
    {
        int64_t t = (*i).second;
        if (GetTime() < t) return; // we've asked recently
    }

    // ask for the xnb info once from the node that sent xnp

    LogPrintf("CXNodeMan::AskForXN - Asking node for missing entry, vin: %s\n", vin.ToString());
    pnode->PushMessage("dseg", vin);
    int64_t askAgain = GetTime() + XNODE_MIN_DSEEP_SECONDS;
    mWeAskedForXNodeListEntry[vin.prevout] = askAgain;
}

void CXNodeMan::Check()
{
    LOCK(cs);

    BOOST_FOREACH(CXNode& xn, vXNodes)
        xn.Check();
}

void CXNodeMan::CheckAndRemove()
{
    LOCK(cs);

    Check();

    //remove inactive
    vector<CXNode>::iterator it = vXNodes.begin();
    while(it != vXNodes.end()){
        if((*it).activeState == CXNode::XNODE_REMOVE || (*it).activeState == CXNode::XNODE_VIN_SPENT || (*it).protocolVersion < nXNodeMinProtocol){
            LogPrint("xnode", "CXNodeMan: Removing inactive xnode %s - %i now\n", (*it).addr.ToString().c_str(), size() - 1);
            it = vXNodes.erase(it);
        } else {
            ++it;
        }
    }

    // check who's asked for the xnode list
    map<CNetAddr, int64_t>::iterator it1 = mAskedUsForXNodeList.begin();
    while(it1 != mAskedUsForXNodeList.end()){
        if((*it1).second < GetTime()) {
            mAskedUsForXNodeList.erase(it1++);
        } else {
            ++it1;
        }
    }

    // check who we asked for the xnode list
    it1 = mWeAskedForXNodeList.begin();
    while(it1 != mWeAskedForXNodeList.end()){
        if((*it1).second < GetTime()){
            mWeAskedForXNodeList.erase(it1++);
        } else {
            ++it1;
        }
    }

    // check which xnodes we've asked for
    map<COutPoint, int64_t>::iterator it2 = mWeAskedForXNodeListEntry.begin();
    while(it2 != mWeAskedForXNodeListEntry.end()){
        if((*it2).second < GetTime()){
            mWeAskedForXNodeListEntry.erase(it2++);
        } else {
            ++it2;
        }
    }

}

void CXNodeMan::Clear()
{
    LOCK(cs);
    vXNodes.clear();
    mAskedUsForXNodeList.clear();
    mWeAskedForXNodeList.clear();
    mWeAskedForXNodeListEntry.clear();
    nDsqCount = 0;
}

int CXNodeMan::CountEnabled(int protocolVersion)
{
    int i = 0;
    protocolVersion = protocolVersion == -1 ? xnodePayments.GetMinXNodePaymentsProto() : protocolVersion;

    BOOST_FOREACH(CXNode& xn, vXNodes) {
        xn.Check();
        if(xn.protocolVersion < protocolVersion || !xn.IsEnabled()) continue;
        i++;
    }

    return i;
}

int CXNodeMan::CountXNodesAboveProtocol(int protocolVersion)
{
    int i = 0;

    BOOST_FOREACH(CXNode& xn, vXNodes) {
        xn.Check();
        if(xn.protocolVersion < protocolVersion || !xn.IsEnabled()) continue;
        i++;
    }

    return i;
}

void CXNodeMan::DsegUpdate(CNode* pnode)
{
    LOCK(cs);

    std::map<CNetAddr, int64_t>::iterator it = mWeAskedForXNodeList.find(pnode->addr);
    if (it != mWeAskedForXNodeList.end())
    {
        if (GetTime() < (*it).second) {
            LogPrintf("dseg - we already asked %s for the list; skipping...\n", pnode->addr.ToString());
            return;
        }
    }
    pnode->PushMessage("dseg", CTxIn());
    int64_t askAgain = GetTime() + XNODES_DSEG_SECONDS;
    mWeAskedForXNodeList[pnode->addr] = askAgain;
}

CXNode *CXNodeMan::Find(const CTxIn &vin)
{
    LOCK(cs);

    BOOST_FOREACH(CXNode& xn, vXNodes)
    {
        if(xn.vin.prevout == vin.prevout)
            return &xn;
    }
    return NULL;
}

CXNode* CXNodeMan::FindOldestNotInVec(const std::vector<CTxIn> &vVins, int nMinimumAge)
{
    LOCK(cs);

    CXNode *pOldestXNode = NULL;

    BOOST_FOREACH(CXNode &xn, vXNodes)
    {   
        xn.Check();
        if(!xn.IsEnabled()) continue;

        if(xn.GetXNodeInputAge() < nMinimumAge) continue;

        bool found = false;
        BOOST_FOREACH(const CTxIn& vin, vVins)
            if(xn.vin.prevout == vin.prevout)
            {   
                found = true;
                break;
            }
        if(found) continue;

        if(pOldestXNode == NULL || pOldestXNode->SecondsSincePayment() < xn.SecondsSincePayment())
        {
            pOldestXNode = &xn;
        }
    }

    return pOldestXNode;
}

CXNode *CXNodeMan::FindRandom()
{
    LOCK(cs);

    if(size() == 0) return NULL;

    return &vXNodes[GetRandInt(vXNodes.size())];
}

CXNode *CXNodeMan::Find(const CPubKey &pubKeyXNode)
{
    LOCK(cs);

    BOOST_FOREACH(CXNode& xn, vXNodes)
    {
        if(xn.pubkey2 == pubKeyXNode)
            return &xn;
    }
    return NULL;
}

CXNode *CXNodeMan::FindRandomNotInVec(std::vector<CTxIn> &vecToExclude, int protocolVersion)
{
    LOCK(cs);

    protocolVersion = protocolVersion == -1 ? xnodePayments.GetMinXNodePaymentsProto() : protocolVersion;

    int nCountEnabled = CountEnabled(protocolVersion);
    LogPrintf("CXNodeMan::FindRandomNotInVec - nCountEnabled - vecToExclude.size() %d\n", nCountEnabled - vecToExclude.size());
    if(nCountEnabled - vecToExclude.size() < 1) return NULL;

    int rand = GetRandInt(nCountEnabled - vecToExclude.size());
    LogPrintf("CXNodeMan::FindRandomNotInVec - rand %d\n", rand);
    bool found;

    BOOST_FOREACH(CXNode &xn, vXNodes) {
        if(xn.protocolVersion < protocolVersion || !xn.IsEnabled()) continue;
        found = false;
        BOOST_FOREACH(CTxIn &usedVin, vecToExclude) {
            if(xn.vin.prevout == usedVin.prevout) {
                found = true;
                break;
            }
        }
        if(found) continue;
        if(--rand < 1) {
            return &xn;
        }
    }

    return NULL;
}

CXNode* CXNodeMan::GetCurrentXnode(int mod, int64_t nBlockHeight, int minProtocol)
{
    unsigned int score = 0;
    CXNode* winner = NULL;

    // scan for winner
    BOOST_FOREACH(CXNode& xn, vXNodes) {
        xn.Check();
        if(xn.protocolVersion < minProtocol || !xn.IsEnabled()) continue;

        // calculate the score for each xnode
        uint256 n = xn.CalculateScore(mod, nBlockHeight);
        unsigned int n2 = 0;
        memcpy(&n2, &n, sizeof(n2));

        // determine the winner
        if(n2 > score){
            score = n2;
            winner = &xn;
        }
    }

    return winner;
}

int CXNodeMan::GetXNodeRank(const CTxIn& vin, int64_t nBlockHeight, int minProtocol, bool fOnlyActive)
{
    std::vector<pair<unsigned int, CTxIn> > vecXNodeScores;

    //make sure we know about this block
    uint256 hash = 0;
    if(!GetBlockHash(hash, nBlockHeight)) return -1;

    // scan for winner
    BOOST_FOREACH(CXNode& xn, vXNodes) {

        if(xn.protocolVersion < minProtocol) continue;
        if(fOnlyActive) {
            xn.Check();
            if(!xn.IsEnabled()) continue;
        }

        uint256 n = xn.CalculateScore(1, nBlockHeight);
        unsigned int n2 = 0;
        memcpy(&n2, &n, sizeof(n2));

        vecXNodeScores.push_back(make_pair(n2, xn.vin));
    }

    sort(vecXNodeScores.rbegin(), vecXNodeScores.rend(), CompareValueOnly());

    int rank = 0;
    BOOST_FOREACH (PAIRTYPE(unsigned int, CTxIn)& s, vecXNodeScores){
        rank++;
        if(s.second == vin) {
            return rank;
        }
    }

    return -1;
}

std::vector<pair<int, CXNode> > CXNodeMan::GetXNodeRanks(int64_t nBlockHeight, int minProtocol)
{
    std::vector<pair<unsigned int, CXNode> > vecXNodeScores;
    std::vector<pair<int, CXNode> > vecXNodeRanks;

    //make sure we know about this block
    uint256 hash = 0;
    if(!GetBlockHash(hash, nBlockHeight)) return vecXNodeRanks;

    // scan for winner
    BOOST_FOREACH(CXNode& xn, vXNodes) {

        xn.Check();

        if(xn.protocolVersion < minProtocol) continue;
        if(!xn.IsEnabled()) {
            continue;
        }

        uint256 n = xn.CalculateScore(1, nBlockHeight);
        unsigned int n2 = 0;
        memcpy(&n2, &n, sizeof(n2));

        vecXNodeScores.push_back(make_pair(n2, xn));
    }

    sort(vecXNodeScores.rbegin(), vecXNodeScores.rend(), CompareValueOnlyXN());

    int rank = 0;
    BOOST_FOREACH (PAIRTYPE(unsigned int, CXNode)& s, vecXNodeScores){
        rank++;
        vecXNodeRanks.push_back(make_pair(rank, s.second));
    }

    return vecXNodeRanks;
}

CXNode* CXNodeMan::GetXNodeByRank(int nRank, int64_t nBlockHeight, int minProtocol, bool fOnlyActive)
{
    std::vector<pair<unsigned int, CTxIn> > vecXNodeScores;

    // scan for winner
    BOOST_FOREACH(CXNode& xn, vXNodes) {

        if(xn.protocolVersion < minProtocol) continue;
        if(fOnlyActive) {
            xn.Check();
            if(!xn.IsEnabled()) continue;
        }

        uint256 n = xn.CalculateScore(1, nBlockHeight);
        unsigned int n2 = 0;
        memcpy(&n2, &n, sizeof(n2));

        vecXNodeScores.push_back(make_pair(n2, xn.vin));
    }

    sort(vecXNodeScores.rbegin(), vecXNodeScores.rend(), CompareValueOnly());

    int rank = 0;
    BOOST_FOREACH (PAIRTYPE(unsigned int, CTxIn)& s, vecXNodeScores){
        rank++;
        if(rank == nRank) {
            return Find(s.second);
        }
    }

    return NULL;
}

void CXNodeMan::ProcessXNodeConnections()
{
    LOCK(cs_vNodes);

    //if(!xnodeEnginePool.pSubmittedToXNode) return;
    
    BOOST_FOREACH(CNode* pnode, vNodes)
    {
        //if(xnodeEnginePool.pSubmittedToXNode->addr == pnode->addr) continue;

        if(pnode->fXNodeConnection){
            LogPrintf("Closing xnode connection %s \n", pnode->addr.ToString().c_str());
            pnode->CloseSocketDisconnect();
        }
    }
}

void CXNodeMan::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{
    // Check for sync
    if(!xnodeSync.IsXNodeSynced()) return;

    LOCK(cs_process_message);

    if (strCommand == "dsee") { //XNodeEngine Election Entry

        CTxIn vin;
        CService addr;
        CPubKey pubkey;
        CPubKey pubkey2;
        vector<unsigned char> vchSig;
        int64_t sigTime;
        int count;
        int current;
        int64_t lastUpdated;
        int protocolVersion;
        CScript donationAddress;
        int donationPercentage;
        std::string strMessage;

        // 70047 and greater
        vRecv >> vin >> addr >> vchSig >> sigTime >> pubkey >> pubkey2 >> count >> current >> lastUpdated >> protocolVersion >> donationAddress >> donationPercentage;

        // make sure signature isn't in the future (past is OK)
        if (sigTime > GetAdjustedTime() + 60 * 60) {
            LogPrintf("dsee - Signature rejected, too far into the future %s\n", vin.ToString().c_str());
            return;
        }

        bool isLocal = addr.IsRFC1918() || addr.IsLocal();
        //if(RegTest()) isLocal = false;

        std::string vchPubKey(pubkey.begin(), pubkey.end());
        std::string vchPubKey2(pubkey2.begin(), pubkey2.end());

        strMessage = addr.ToString() + boost::lexical_cast<std::string>(sigTime) + vchPubKey + vchPubKey2 + boost::lexical_cast<std::string>(protocolVersion)  + donationAddress.ToString() + boost::lexical_cast<std::string>(donationPercentage);

        if(donationPercentage < 0 || donationPercentage > 100){
            LogPrintf("dsee - donation percentage out of range %d\n", donationPercentage);
            return;     
        }
        if(protocolVersion < MIN_PEER_PROTO_VERSION) {
            LogPrintf("dsee - ignoring outdated xnode %s protocol version %d\n", vin.ToString().c_str(), protocolVersion);
            return;
        }

        CScript pubkeyScript;
        pubkeyScript.SetDestination(pubkey.GetID());

        if(pubkeyScript.size() != 25) {
            LogPrintf("dsee - pubkey the wrong size\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        CScript pubkeyScript2;
        pubkeyScript2.SetDestination(pubkey2.GetID());

        if(pubkeyScript2.size() != 25) {
            LogPrintf("dsee - pubkey2 the wrong size\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        if(!vin.scriptSig.empty()) {
            LogPrintf("dsee - Ignore Not Empty ScriptSig %s\n",vin.ToString().c_str());
            return;
        }

        std::string errorMessage = "";
        if(!xnodeEngineSigner.VerifyMessage(pubkey, vchSig, strMessage, errorMessage)){
            LogPrintf("dsee - Got bad xnode address signature\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        //search existing xnode list, this is where we update existing xnodes with new dsee broadcasts
        CXNode* pxn = this->Find(vin);
        // if we are a xnode but with undefined vin and this dsee is ours (matches our XNode privkey) then just skip this part
        if(pxn != NULL && !(fXnode && activeXNode.vin == CTxIn() && pubkey2 == activeXNode.pubKeyXNode))
        {
            // count == -1 when it's a new entry
            //   e.g. We don't want the entry relayed/time updated when we're syncing the list
            // xn.pubkey = pubkey, IsVinAssociatedWithPubkey is validated once below,
            //   after that they just need to match
            if(count == -1 && pxn->pubkey == pubkey && !pxn->UpdatedWithin(XNODE_MIN_DSEE_SECONDS)){
                pxn->UpdateLastSeen();

                if(pxn->sigTime < sigTime){ //take the newest entry
                    LogPrintf("dsee - Got updated entry for %s\n", addr.ToString().c_str());
                    pxn->pubkey2 = pubkey2;
                    pxn->sigTime = sigTime;
                    pxn->sig = vchSig;
                    pxn->protocolVersion = protocolVersion;
                    pxn->addr = addr;
                    pxn->donationAddress = donationAddress;
                    pxn->donationPercentage = donationPercentage;
                    pxn->Check();
                    if(pxn->IsEnabled())
                        xnodeman.RelayXNodeEntry(vin, addr, vchSig, sigTime, pubkey, pubkey2, count, current, lastUpdated, protocolVersion, donationAddress, donationPercentage);
                }
            }

            return;
        }

        // make sure the vout that was signed is related to the transaction that spawned the xnode
        //  - this is expensive, so it's only done once per xnode
        if(!xnodeEngineSigner.IsVinAssociatedWithPubkey(vin, pubkey)) {
            LogPrintf("dsee - Got mismatched pubkey and vin\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        LogPrint("xnode", "dsee - Got NEW xnode entry %s\n", addr.ToString().c_str());

        // make sure it's still unspent
        //  - this is checked later by .check() in many places and by ThreadCheckXNodeEnginePool()

        CValidationState state;
        CTransaction tx = CTransaction();
        //CTxOut vout = CTxOut(XNODEENGINE_POOL_MAX, xnodeEnginePool.collateralPubKey);
        //tx.vin.push_back(vin);
        //tx.vout.push_back(vout);
        bool fAcceptable = false;
        {
            TRY_LOCK(cs_main, lockMain);
            if(!lockMain) return;
            fAcceptable = AcceptableInputs(mempool, tx, false, NULL);
        }
        if(fAcceptable){
            LogPrint("xnode", "dsee - Accepted xnode entry %i %i\n", count, current);

            if(GetInputAge(vin) < XNODE_MIN_CONFIRMATIONS){
                LogPrintf("dsee - Input must have least %d confirmations\n", XNODE_MIN_CONFIRMATIONS);
                Misbehaving(pfrom->GetId(), 20);
                return;
            }

            // verify that sig time is legit in past
            // should be at least not earlier than block when 5,000 ESP tx got XNODE_MIN_CONFIRMATIONS
            uint256 hashBlock = 0;
            GetTransaction(vin.prevout.hash, tx, hashBlock);
            map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
           if (mi != mapBlockIndex.end() && (*mi).second)
            {
                CBlockIndex* pXNIndex = (*mi).second; // block for 5,000 ESP tx -> 1 confirmation
                CBlockIndex* pConfIndex = FindBlockByHeight((pXNIndex->nHeight + XNODE_MIN_CONFIRMATIONS - 1)); // block where tx got XNODE_MIN_CONFIRMATIONS
                if(pConfIndex->GetBlockTime() > sigTime)
                {
                    LogPrintf("dsee - Bad sigTime %d for xnode %20s %105s (%i conf block is at %d)\n",
                              sigTime, addr.ToString(), vin.ToString(), XNODE_MIN_CONFIRMATIONS, pConfIndex->GetBlockTime());
                    return;
                }
            }


            // use this as a peer
            addrman.Add(CAddress(addr), pfrom->addr, 2*60*60);

            //doesn't support multisig addresses
            if(donationAddress.IsPayToScriptHash()){
                donationAddress = CScript();
                donationPercentage = 0;
            }

            // add our xnode
            CXNode xn(addr, vin, pubkey, vchSig, sigTime, pubkey2, protocolVersion, donationAddress, donationPercentage);
            xn.UpdateLastSeen(lastUpdated);
            this->Add(xn);

            // if it matches our xnodeprivkey, then we've been remotely activated
            if(pubkey2 == activeXNode.pubKeyXNode && protocolVersion == PROTOCOL_VERSION){
                activeXNode.EnableHotColdXnode(vin, addr);
            }

            if(count == -1 && !isLocal)
                xnodeman.RelayXNodeEntry(vin, addr, vchSig, sigTime, pubkey, pubkey2, count, current, lastUpdated, protocolVersion, donationAddress, donationPercentage);

        } else {
            LogPrintf("dsee - Rejected xnode entry %s\n", addr.ToString().c_str());

            int nDoS = 0;
            if (state.IsInvalid(nDoS))
            {
                LogPrintf("dsee - %s from %s %s was not accepted into the memory pool\n", tx.GetHash().ToString().c_str(),
                    pfrom->addr.ToString().c_str(), pfrom->cleanSubVer.c_str());
                if (nDoS > 0)
                    Misbehaving(pfrom->GetId(), nDoS);
            }
        }
    }

    else if (strCommand == "dseep") { //XNodeEngine Election Entry Ping

        CTxIn vin;
        vector<unsigned char> vchSig;
        int64_t sigTime;
        bool stop;
        vRecv >> vin >> vchSig >> sigTime >> stop;

        //LogPrintf("dseep - Received: vin: %s sigTime: %lld stop: %s\n", vin.ToString().c_str(), sigTime, stop ? "true" : "false");

        if (sigTime > GetAdjustedTime() + 60 * 60) {
            LogPrintf("dseep - Signature rejected, too far into the future %s\n", vin.ToString().c_str());
            return;
        }

        if (sigTime <= GetAdjustedTime() - 60 * 60) {
            LogPrintf("dseep - Signature rejected, too far into the past %s - %d %d \n", vin.ToString().c_str(), sigTime, GetAdjustedTime());
            return;
        }

        // see if we have this xnode
        CXNode* pxn = this->Find(vin);
        if(pxn != NULL && pxn->protocolVersion >= MIN_PEER_PROTO_VERSION)
        {
            // LogPrintf("dseep - Found corresponding xn for vin: %s\n", vin.ToString().c_str());
            // take this only if it's newer
            if(pxn->lastDseep < sigTime)
            {
                std::string strMessage = pxn->addr.ToString() + boost::lexical_cast<std::string>(sigTime) + boost::lexical_cast<std::string>(stop);

                std::string errorMessage = "";
                if(!xnodeEngineSigner.VerifyMessage(pxn->pubkey2, vchSig, strMessage, errorMessage))
                {
                    LogPrintf("dseep - Got bad xnode address signature %s \n", vin.ToString().c_str());
                    //Misbehaving(pfrom->GetId(), 100); LEAVE DISABLED
                    return;
                }

                pxn->lastDseep = sigTime;

                if(!pxn->UpdatedWithin(XNODE_MIN_DSEEP_SECONDS))
                {
                    if(stop) pxn->Disable();
                    else
                    {
                        pxn->UpdateLastSeen();
                        pxn->Check();
                        if(!pxn->IsEnabled()) return;
                    }
                    xnodeman.RelayXNodeEntryPing(vin, vchSig, sigTime, stop);
                }
            }
            return;
        }

        LogPrint("xnode", "dseep - Couldn't find xnode entry %s\n", vin.ToString().c_str());

        std::map<COutPoint, int64_t>::iterator i = mWeAskedForXNodeListEntry.find(vin.prevout);
        if (i != mWeAskedForXNodeListEntry.end())
        {
            int64_t t = (*i).second;
            if (GetTime() < t) return; // we've asked recently
        }

        // ask for the dsee info once from the node that sent dseep

        LogPrintf("dseep - Asking source node for missing entry %s\n", vin.ToString().c_str());
        pfrom->PushMessage("dseg", vin);
        int64_t askAgain = GetTime()+ XNODE_MIN_DSEEP_SECONDS;
        mWeAskedForXNodeListEntry[vin.prevout] = askAgain;

    } else if (strCommand == "mvote") { //XNode Vote

        CTxIn vin;
        vector<unsigned char> vchSig;
        int nVote;
        vRecv >> vin >> vchSig >> nVote;

        // see if we have this XNode
        CXNode* pxn = this->Find(vin);
        if(pxn != NULL)
        {
            if((GetAdjustedTime() - pxn->lastVote) > (60*60))
            {
                std::string strMessage = vin.ToString() + boost::lexical_cast<std::string>(nVote);

                std::string errorMessage = "";
                if(!xnodeEngineSigner.VerifyMessage(pxn->pubkey2, vchSig, strMessage, errorMessage))
                {
                    LogPrintf("mvote - Got bad XNode address signature %s \n", vin.ToString().c_str());
                    return;
                }

                pxn->nVote = nVote;
                pxn->lastVote = GetAdjustedTime();

                //send to all peers
                LOCK(cs_vNodes);
                BOOST_FOREACH(CNode* pnode, vNodes)
                    pnode->PushMessage("mvote", vin, vchSig, nVote);
            }

            return;
        }

    } else if (strCommand == "dseg") { //Get xnode list or specific entry

        CTxIn vin;
        vRecv >> vin;

        if(vin == CTxIn()) { //only should ask for this once
            //local network
            if(!pfrom->addr.IsRFC1918() && Params().NetworkID() == CChainParams::MAIN)
            {
                std::map<CNetAddr, int64_t>::iterator i = mAskedUsForXNodeList.find(pfrom->addr);
                if (i != mAskedUsForXNodeList.end())
                {
                    int64_t t = (*i).second;
                    if (GetTime() < t) {
                        Misbehaving(pfrom->GetId(), 34);
                        LogPrintf("dseg - peer already asked me for the list\n");
                        return;
                    }
                }

                int64_t askAgain = GetTime() + XNODES_DSEG_SECONDS;
                mAskedUsForXNodeList[pfrom->addr] = askAgain;
            }
        } //else, asking for a specific node which is ok

        int count = this->size();
        int i = 0;

        BOOST_FOREACH(CXNode& xn, vXNodes) {

            if(xn.addr.IsRFC1918()) continue; //local network

            if(xn.IsEnabled())
            {
                LogPrint("xnode", "dseg - Sending xnode entry - %s \n", xn.addr.ToString().c_str());
                if(vin == CTxIn()){
                    pfrom->PushMessage("dsee", xn.vin, xn.addr, xn.sig, xn.sigTime, xn.pubkey, xn.pubkey2, count, i, xn.lastTimeSeen, xn.protocolVersion, xn.donationAddress, xn.donationPercentage);
                } else if (vin == xn.vin) {
                    pfrom->PushMessage("dsee", xn.vin, xn.addr, xn.sig, xn.sigTime, xn.pubkey, xn.pubkey2, count, i, xn.lastTimeSeen, xn.protocolVersion, xn.donationAddress, xn.donationPercentage);
                    LogPrintf("dseg - Sent 1 xnode entries to %s\n", pfrom->addr.ToString().c_str());
                    return;
                }
                i++;
            }
        }

        LogPrintf("dseg - Sent %d xnode entries to %s\n", i, pfrom->addr.ToString().c_str());
    }

}

void CXNodeMan::RelayXNodeEntry(const CTxIn vin, const CService addr, const std::vector<unsigned char> vchSig, const int64_t nNow, const CPubKey pubkey, const CPubKey pubkey2, const int count, const int current, const int64_t lastUpdated, const int protocolVersion, CScript donationAddress, int donationPercentage)
{
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
        pnode->PushMessage("dsee", vin, addr, vchSig, nNow, pubkey, pubkey2, count, current, lastUpdated, protocolVersion, donationAddress, donationPercentage);
}

void CXNodeMan::RelayXNodeEntryPing(const CTxIn vin, const std::vector<unsigned char> vchSig, const int64_t nNow, const bool stop)
{
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
        pnode->PushMessage("dseep", vin, vchSig, nNow, stop);
}

void CXNodeMan::Remove(CTxIn vin)
{
    LOCK(cs);

    vector<CXNode>::iterator it = vXNodes.begin();
    while(it != vXNodes.end()){
        if((*it).vin == vin){
            LogPrint("xnode", "CXNodeMan: Removing XNode %s - %i now\n", (*it).addr.ToString().c_str(), size() - 1);
            vXNodes.erase(it);
            break;
        }
    }
}

std::string CXNodeMan::ToString() const
{
    std::ostringstream info;

    info << "xnodes: " << (int)vXNodes.size() <<
            ", peers who asked us for xnode list: " << (int)mAskedUsForXNodeList.size() <<
            ", peers we asked for xnode list: " << (int)mWeAskedForXNodeList.size() <<
            ", entries in XNode list we asked for: " << (int)mWeAskedForXNodeListEntry.size() <<
            ", nDsqCount: " << (int)nDsqCount;

    return info.str();
}
