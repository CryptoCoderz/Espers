// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2017-2021 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xnodereward.h"
#include "xnodemngr.h"
#include "xnodecrypter.h"
#include "util/util.h"
#include "core/sync.h"
#include "consensus/fork.h"
#include "node/addrman.h"
#include "util/reverse_iterator.h"

#include <boost/lexical_cast.hpp>

CCriticalSection cs_xnodepayments;

/** Object for who's going to get paid on which blocks */
CXNodePayments xnodePayments;
// keep track of XNode votes I've seen
map<uint256, CXNodePaymentWinner> mapSeenXNodeVotes;

//
// CXNodePaymentDB
//

CXNodePaymentDB::CXNodePaymentDB()
{
    pathDB = GetDataDir() / "xnoderewards.dat";
    strMagicMessage = "XNodePayments";
}

bool CXNodePaymentDB::Write(const CXNodePayments& objToSave)
{
    int64_t nStart = GetTimeMillis();

    // serialize, checksum data up to that point, then append checksum
    CDataStream ssObj(SER_DISK, CLIENT_VERSION);
    ssObj << strMagicMessage; // xnode cache file specific magic message
    ssObj << FLATDATA(Params().MessageStart()); // network specific magic number
    ssObj << objToSave;
    uint256 hash = Hash(ssObj.begin(), ssObj.end());
    ssObj << hash;

    // open output file, and associate with CAutoFile
    FILE *file = fopen(pathDB.string().c_str(), "wb");
    CAutoFile fileout(file, SER_DISK, CLIENT_VERSION);
    if (!fileout)
        return error("%s : Failed to open file %s", __func__, pathDB.string());

    // Write and commit header, data
    try {
        fileout << ssObj;
    }
    catch (std::exception &e) {
        return error("%s : Serialize or I/O error - %s", __func__, e.what());
    }
    fileout.fclose();

    LogPrintf("Written info to xnoderewards.dat  %dms\n", GetTimeMillis() - nStart);

    return true;
}

CXNodePaymentDB::ReadResult CXNodePaymentDB::Read(CXNodePayments& objToLoad)
{

    int64_t nStart = GetTimeMillis();
    // open input file, and associate with CAutoFile
    FILE *file = fopen(pathDB.string().c_str(), "rb");
    CAutoFile filein(file, SER_DISK, CLIENT_VERSION);
    if (!filein)
    {
        error("%s : Failed to open file %s", __func__, pathDB.string());
        return FileError;
    }

    // use file size to size memory buffer
    int fileSize = boost::filesystem::file_size(pathDB);
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

    CDataStream ssObj(vchData, SER_DISK, CLIENT_VERSION);

    // verify stored checksum matches input data
    uint256 hashTmp = Hash(ssObj.begin(), ssObj.end());
    if (hashIn != hashTmp)
    {
        error("%s : Checksum mismatch, data corrupted", __func__);
        return IncorrectHash;
    }


    unsigned char pchMsgTmp[4];
    std::string strMagicMessageTmp;
    try {
        // de-serialize file header (xnode cache file specific magic message) and ..
        ssObj >> strMagicMessageTmp;

        // ... verify the message matches predefined one
        if (strMagicMessage != strMagicMessageTmp)
        {
            error("%s : Invalid xnode payement cache magic message", __func__);
            return IncorrectMagicMessage;
        }

        // de-serialize file header (network specific magic number) and ..
        ssObj >> FLATDATA(pchMsgTmp);

        // ... verify the network matches ours
        if (memcmp(pchMsgTmp, Params().MessageStart(), sizeof(pchMsgTmp)))
        {
            error("%s : Invalid network magic number", __func__);
            return IncorrectMagicNumber;
        }

        // de-serialize data into CXNodePayments object
        ssObj >> objToLoad;
    }
    catch (std::exception &e) {
        objToLoad.Clear();
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return IncorrectFormat;
    }

    LogPrintf("Loaded info from xnoderewards.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrintf("  %s\n", objToLoad.ToString());

    LogPrintf("XNode payments manager - cleaning....\n");
    objToLoad.CleanPaymentList(); // clean out expired
    LogPrintf("XNode payments manager - result:\n");
    LogPrintf("  %s\n", objToLoad.ToString());


    return Ok;
}

void DumpXNodePayments()
{
    int64_t nStart = GetTimeMillis();

    CXNodePaymentDB paymentdb;
    CXNodePayments tempPayments;

    LogPrintf("Verifying xnoderewards.dat format...\n");
    CXNodePaymentDB::ReadResult readResult = paymentdb.Read(tempPayments);
    // there was an error and it was not an error on file opening => do not proceed
    if (readResult == CXNodePaymentDB::FileError)
        LogPrintf("Missing rewards file - xnoderewards.dat, will try to recreate\n");
    else if (readResult != CXNodePaymentDB::Ok)
    {
        LogPrintf("Error reading xnoderewards.dat: ");
        if(readResult == CXNodePaymentDB::IncorrectFormat)
            LogPrintf("magic is ok but data has invalid format, will try to recreate\n");
        else
        {
            LogPrintf("file format is unknown or invalid, please fix it manually\n");
            return;
        }
    }
    LogPrintf("Writting info to xnoderewards.dat...\n");
    paymentdb.Write(xnodePayments);

    LogPrintf("Reward dump finished  %dms\n", GetTimeMillis() - nStart);
}

int CXNodePayments::GetMinXNodePaymentsProto() {
    return IsSporkActive(SPORK_10_XNODE_PAY_UPDATED_NODES)
            ? MIN_XNODE_PAYMENT_PROTO_VERSION_2
            : MIN_XNODE_PAYMENT_PROTO_VERSION_1;
}

void ProcessMessageXNodePayments(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{
    if(!xnodeSync.IsXNodeSynced()) {
        LogPrintf("ProcessMessageXNodePayments - xnodeSync failed!\n");
        return;
    }

    if (strCommand == "xnget") { //XNode Payments Request Sync

        if(pfrom->HasFulfilledRequest("xnget")) {
            LogPrintf("xnget - peer already asked me for the list\n");
            Misbehaving(pfrom->GetId(), 20);
            return;
        }

        pfrom->FulfilledRequest("xnget");
        xnodePayments.Sync(pfrom);
        LogPrintf("xnget - Sent XNode winners to %s\n", pfrom->addr.ToString().c_str());
    }
    else if (strCommand == "xnw") { //XNode Payments Declare Winner

        LOCK(cs_xnodepayments);

        //this is required in litemode
        CXNodePaymentWinner winner;
        vRecv >> winner;

        if(pindexBest == NULL) return;

        CTxDestination address1;
        ExtractDestination(winner.payee, address1);
        //CXNodeAddress address2(address1);
        CBitcoinAddress address2(address1);

        uint256 hash = winner.GetHash();
        if(mapSeenXNodeVotes.count(hash)) {
            if(fDebug) LogPrintf("xnw - seen vote %s Addr %s Height %d bestHeight %d\n", hash.ToString().c_str(), address2.ToString().c_str(), winner.nBlockHeight, pindexBest->nHeight);
            return;
        }

        if(winner.nBlockHeight < pindexBest->nHeight - 10 || winner.nBlockHeight > pindexBest->nHeight+20){
            LogPrintf("xnw - winner out of range %s Addr %s Height %d bestHeight %d\n", winner.vin.ToString().c_str(), address2.ToString().c_str(), winner.nBlockHeight, pindexBest->nHeight);
            return;
        }

        if(winner.vin.nSequence != std::numeric_limits<unsigned int>::max()){
            LogPrintf("xnw - invalid nSequence\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        LogPrintf("xnw - winning vote - Vin %s Addr %s Height %d bestHeight %d\n", winner.vin.ToString().c_str(), address2.ToString().c_str(), winner.nBlockHeight, pindexBest->nHeight);

        if(!xnodePayments.CheckSignature(winner)){
            LogPrintf("xnw - invalid signature\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        mapSeenXNodeVotes.insert(make_pair(hash, winner));

        if(xnodePayments.AddWinningXNode(winner)){
            xnodePayments.Relay(winner);
        }
    }
}


bool CXNodePayments::CheckSignature(CXNodePaymentWinner& winner)
{
    //note: need to investigate why this is failing
    std::string strMessage = winner.vin.ToString().c_str() + boost::lexical_cast<std::string>(winner.nBlockHeight) + winner.payee.ToString();
    std::string strPubKey = strMainPubKey ;
    CPubKey pubkey(ParseHex(strPubKey));

    std::string errorMessage = "";
    if(!xnodeEngineSigner.VerifyMessage(pubkey, winner.vchSig, strMessage, errorMessage)){
        return false;
    }

    return true;
}

bool CXNodePayments::Sign(CXNodePaymentWinner& winner)
{
    std::string strMessage = winner.vin.ToString().c_str() + boost::lexical_cast<std::string>(winner.nBlockHeight) + winner.payee.ToString();

    CKey key2;
    CPubKey pubkey2;
    std::string errorMessage = "";

    if(!xnodeEngineSigner.SetKey(strMasterPrivKey, errorMessage, key2, pubkey2))
    {
        //LogPrintf("CXNodePayments::Sign - ERROR: Invalid XNodeprivkey: '%s'\n", errorMessage.c_str());
        //return false;
        LogPrintf("CXNodePayments::Sign - Sign XNodeprivkey bypassed");
    }

    if(!xnodeEngineSigner.SignMessage(strMessage, errorMessage, winner.vchSig, key2)) {
        //LogPrintf("CXNodePayments::Sign - Sign message failed");
        //return false;
        LogPrintf("CXNodePayments::Sign - Sign message bypassed");
    }

    if(!xnodeEngineSigner.VerifyMessage(pubkey2, winner.vchSig, strMessage, errorMessage)) {
        //LogPrintf("CXNodePayments::Sign - Verify message failed");
        //return false;
        LogPrintf("CXNodePayments::Sign - Verify message bypassed");
    }

    return true;
}

void CXNodePayments::Clear()
{
    mapSeenXNodeVotes.clear();
}

uint64_t CXNodePayments::CalculateScore(uint256 blockHash, CTxIn& vin)
{
    uint256 n1 = blockHash;
    uint256 n2 = Hash(BEGIN(n1), END(n1));
    uint256 n3 = Hash(BEGIN(vin.prevout.hash), END(vin.prevout.hash));
    uint256 n4 = n3 > n2 ? (n3 - n2) : (n2 - n3);

    //printf(" -- CXNodePayments CalculateScore() n2 = %d \n", n2.Get64());
    //printf(" -- CXNodePayments CalculateScore() n3 = %d \n", n3.Get64());
    //printf(" -- CXNodePayments CalculateScore() n4 = %d \n", n4.Get64());

    return n4.Get64();
}

bool CXNodePayments::GetBlockPayee(int nBlockHeight, CScript& payee, CTxIn& vin)
{
    for (CXNodePaymentWinner& winner : vWinning){
        if(winner.nBlockHeight == nBlockHeight) {
            payee = winner.payee;
            vin = winner.vin;
            return true;
        }
    }

    return false;
}

bool CXNodePayments::GetWinningXNode(int nBlockHeight, CTxIn& vinOut)
{
    for (CXNodePaymentWinner& winner : vWinning){
        if(winner.nBlockHeight == nBlockHeight) {
            vinOut = winner.vin;
            return true;
        }
    }

    return false;
}

bool CXNodePayments::AddWinningXNode(CXNodePaymentWinner& winnerIn)
{
    uint256 blockHash = 0;
    if(!GetBlockHash(blockHash, winnerIn.nBlockHeight-576)) {
        return false;
    }

    winnerIn.score = CalculateScore(blockHash, winnerIn.vin);

    bool foundBlock = false;
    for (CXNodePaymentWinner& winner : vWinning){
        if(winner.nBlockHeight == winnerIn.nBlockHeight) {
            foundBlock = true;
            if(winner.score < winnerIn.score){
                winner.score = winnerIn.score;
                winner.vin = winnerIn.vin;
                winner.payee = winnerIn.payee;
                winner.vchSig = winnerIn.vchSig;

                mapSeenXNodeVotes.insert(make_pair(winnerIn.GetHash(), winnerIn));

                return true;
            }
        }
    }

    // if it's not in the vector
    if(!foundBlock){
        vWinning.push_back(winnerIn);
        mapSeenXNodeVotes.insert(make_pair(winnerIn.GetHash(), winnerIn));

        return true;
    }

    return false;
}

void CXNodePayments::CleanPaymentList()
{
    LOCK(cs_xnodepayments);

    if(pindexBest == NULL) return;

    int nLimit = std::max(((int)xnodeman.size())*((int)1.25), 1000);

    vector<CXNodePaymentWinner>::iterator it;
    for(it=vWinning.begin();it<vWinning.end();it++){
        if(pindexBest->nHeight - (*it).nBlockHeight > nLimit){
            if(fDebug) LogPrintf("CXNodePayments::CleanPaymentList - Removing old XNode payment - block %d\n", (*it).nBlockHeight);
            vWinning.erase(it);
            break;
        }
    }
}

bool CXNodePayments::ProcessBlock(int nBlockHeight)
{
    LOCK(cs_xnodepayments);

    if(nBlockHeight <= nLastBlockHeight) return false;
    if(!enabled) return false;
    CXNodePaymentWinner newWinner;
    int nMinimumAge = xnodeman.CountEnabled();
    CScript payeeSource;

    uint256 hash;
    if(!GetBlockHash(hash, nBlockHeight-10)) return false;
    unsigned int nHash;
    memcpy(&nHash, &hash, 2);

    LogPrintf(" ProcessBlock Start nHeight %d - vin %s. \n", nBlockHeight, activeXNode.vin.ToString().c_str());

    std::vector<CTxIn> vecLastPayments;
    for (CXNodePaymentWinner& winner : reverse_iterate(vWinning))
    {
        //if we already have the same vin - we have one full payment cycle, break
        if(vecLastPayments.size() > (unsigned int)nMinimumAge) break;
        vecLastPayments.push_back(winner.vin);
    }

    // pay to the oldest XN that still had no payment but its input is old enough and it was active long enough
    CXNode *pxn = xnodeman.FindOldestNotInVec(vecLastPayments, nMinimumAge);
    if(pxn != NULL)
    {
        LogPrintf(" Found by FindOldestNotInVec \n");

        newWinner.score = 0;
        newWinner.nBlockHeight = nBlockHeight;
        newWinner.vin = pxn->vin;

        if(pxn->donationPercentage > 0 && (nHash % 100) <= (unsigned int)pxn->donationPercentage) {
            newWinner.payee = pxn->donationAddress;
        } else {
            newWinner.payee = GetScriptForDestination(pxn->pubkey.GetID());
        }

        payeeSource = GetScriptForDestination(pxn->pubkey.GetID());
    }

    //if we can't find new XN to get paid, pick first active XN counting back from the end of vecLastPayments list
    if(newWinner.nBlockHeight == 0 && nMinimumAge > 0)
    {
        LogPrintf(" Find by reverse \n");

        for (CTxIn& vinLP : reverse_iterate(vecLastPayments))
        {
            CXNode* pxn = xnodeman.Find(vinLP);
            if(pxn != NULL)
            {
                pxn->Check();
                if(!pxn->IsEnabled()) continue;

                newWinner.score = 0;
                newWinner.nBlockHeight = nBlockHeight;
                newWinner.vin = pxn->vin;

                if(pxn->donationPercentage > 0 && (nHash % 100) <= (unsigned int)pxn->donationPercentage) {
                    newWinner.payee = pxn->donationAddress;
                } else {
                    newWinner.payee = GetScriptForDestination(pxn->pubkey.GetID());
                }

                payeeSource = GetScriptForDestination(pxn->pubkey.GetID());

                break; // we found active XN
            }
        }
    }

    if(newWinner.nBlockHeight == 0) return false;

    CTxDestination address1;
    ExtractDestination(newWinner.payee, address1);
    //CXNodeAddress address2(address1);
    CBitcoinAddress address2(address1);

    CTxDestination address3;
    ExtractDestination(payeeSource, address3);
    //CXNodeAddress address4(address3);
    CBitcoinAddress address4(address3);

    LogPrintf("Winner payee %s nHeight %d vin source %s. \n", address2.ToString().c_str(), newWinner.nBlockHeight, address4.ToString().c_str());

    if(Sign(newWinner))
    {
        if(AddWinningXNode(newWinner))
        {
            Relay(newWinner);
            nLastBlockHeight = nBlockHeight;
            return true;
        }
    }

    return false;
}


void CXNodePayments::Relay(CXNodePaymentWinner& winner)
{
    CInv inv(MSG_XNODE_WINNER, winner.GetHash());

    vector<CInv> vInv;
    vInv.push_back(inv);
    LOCK(cs_vNodes);
    for (CNode* pnode : vNodes){
        pnode->PushMessage("inv", vInv);
    }
}

void CXNodePayments::Sync(CNode* node)
{
    LOCK(cs_xnodepayments);

    for (CXNodePaymentWinner& winner : vWinning)
        if(winner.nBlockHeight >= pindexBest->nHeight-10 && winner.nBlockHeight <= pindexBest->nHeight + 20)
            node->PushMessage("xnw", winner);
}


bool CXNodePayments::SetPrivKey(std::string strPrivKey)
{
    CXNodePaymentWinner winner;

    // Test signing successful, proceed
    strMasterPrivKey = strPrivKey;

    Sign(winner);

    if(CheckSignature(winner)){
        LogPrintf("CXNodePayments::SetPrivKey - Successfully initialized as XNode payments master\n");
        enabled = true;
        return true;
    } else {
        return false;
    }
}

std::string CXNodePayments::ToString() const
{
    std::ostringstream info;

    info << "xnode rewards: ";

    return info.str();
}
