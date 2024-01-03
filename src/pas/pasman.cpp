// Copyright (c) 2022-2024 The CryptoCoderz Team / Espers project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pasman.h"
#include "pas.h"
#include "pasengine.h"
#include "core/chain.h"
#include "util/util.h"
#include "node/addrman.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>


/** Pubkeyaliasservice manager */
CPubkeyaliasserviceMan paserviceman;
CCriticalSection cs_process_messagePAS;

//
// CPubkeyaliasserviceDB
//

CPubkeyaliasserviceDB::CPubkeyaliasserviceDB()
{
    pathPAS = GetDataDir() / "pascache.dat";
    strMagicMessage = "PubkeyaliasserviceCache";
}

bool CPubkeyaliasserviceDB::Write(const CPubkeyaliasserviceMan& paservicemanToSave)
{
    int64_t nStart = GetTimeMillis();

    // serialize addresses, checksum data up to that point, then append csum
    CDataStream ssPubkeyaliasservices(SER_DISK, CLIENT_VERSION);
    ssPubkeyaliasservices << strMagicMessage; // pubkeyaliasservice cache file specific magic message
    ssPubkeyaliasservices << FLATDATA(Params().MessageStart()); // network specific magic number
    ssPubkeyaliasservices << paservicemanToSave;
    uint256 hash = Hash(ssPubkeyaliasservices.begin(), ssPubkeyaliasservices.end());
    ssPubkeyaliasservices << hash;

    // open output file, and associate with CAutoFile
    FILE *file = fopen(pathPAS.string().c_str(), "wb");
    CAutoFile fileout = CAutoFile(file, SER_DISK, CLIENT_VERSION);
    if (!fileout)
        return error("%s : Failed to open file %s", __func__, pathPAS.string());

    // Write and commit header, data
    try {
        fileout << ssPubkeyaliasservices;
    }
    catch (std::exception &e) {
        return error("%s : Serialize or I/O error - %s", __func__, e.what());
    }
    FileCommit(fileout);
    fileout.fclose();

    LogPrintf("Written info to pascache.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrintf("  %s\n", paservicemanToSave.ToString());

    return true;
}

CPubkeyaliasserviceDB::ReadResult CPubkeyaliasserviceDB::Read(CPubkeyaliasserviceMan& paservicemanToLoad)
{
    int64_t nStart = GetTimeMillis();
    // open input file, and associate with CAutoFile
    FILE *file = fopen(pathPAS.string().c_str(), "rb");
    CAutoFile filein = CAutoFile(file, SER_DISK, CLIENT_VERSION);
    if (!filein)
    {
        error("%s : Failed to open file %s", __func__, pathPAS.string());
        return FileError;
    }

    // use file size to size memory buffer
    int fileSize = boost::filesystem::file_size(pathPAS);
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

    CDataStream ssPubkeyaliasservices(vchData, SER_DISK, CLIENT_VERSION);

    // verify stored checksum matches input data
    uint256 hashTmp = Hash(ssPubkeyaliasservices.begin(), ssPubkeyaliasservices.end());
    if (hashIn != hashTmp)
    {
        error("%s : Checksum mismatch, data corrupted", __func__);
        return IncorrectHash;
    }

    unsigned char pchMsgTmp[4];
    std::string strMagicMessageTmp;
    try {
        // de-serialize file header (pubkeyaliasservice cache file specific magic message) and ..

        ssPubkeyaliasservices >> strMagicMessageTmp;

        // ... verify the message matches predefined one
        if (strMagicMessage != strMagicMessageTmp)
        {
            error("%s : Invalid pubkeyaliasservice cache magic message", __func__);
            return IncorrectMagicMessage;
        }

        // de-serialize file header (network specific magic number) and ..
        ssPubkeyaliasservices >> FLATDATA(pchMsgTmp);

        // ... verify the network matches ours
        if (memcmp(pchMsgTmp, Params().MessageStart(), sizeof(pchMsgTmp)))
        {
            error("%s : Invalid network magic number", __func__);
            return IncorrectMagicNumber;
        }

        // de-serialize address data into one CPasList object
        ssPubkeyaliasservices >> paservicemanToLoad;
    }
    catch (std::exception &e) {
        paservicemanToLoad.ClearPAS();
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return IncorrectFormat;
    }

    paservicemanToLoad.CheckAndRemovePAS(); // clean out expired
    LogPrintf("Loaded info from pascache.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrintf("  %s\n", paservicemanToLoad.ToString());

    return Ok;
}

void DumpPubkeyaliasservices()
{
    int64_t nStart = GetTimeMillis();

    CPubkeyaliasserviceDB pasdb;
    CPubkeyaliasserviceMan tempPAserviceman;

    LogPrintf("Verifying pascache.dat format...\n");
    CPubkeyaliasserviceDB::ReadResult readResult = pasdb.Read(tempPAserviceman);
    // there was an error and it was not an error on file openning => do not proceed
    if (readResult == CPubkeyaliasserviceDB::FileError)
        LogPrintf("Missing pubkeyaliasservice list file - pascache.dat, will try to recreate\n");
    else if (readResult != CPubkeyaliasserviceDB::Ok)
    {
        LogPrintf("Error reading pascache.dat: ");
        if(readResult == CPubkeyaliasserviceDB::IncorrectFormat)
            LogPrintf("magic is ok but data has invalid format, will try to recreate\n");
        else
        {
            LogPrintf("file format is unknown or invalid, please fix it manually\n");
            return;
        }
    }
    LogPrintf("Writting info to pascache.dat...\n");
    pasdb.Write(paserviceman);

    LogPrintf("Pubkeyaliasservice dump finished  %dms\n", GetTimeMillis() - nStart);
}

CPubkeyaliasserviceMan::CPubkeyaliasserviceMan() {
    // Do nothing
}

bool CPubkeyaliasserviceMan::AddPAS(CPubkeyaliasservice &pas)
{
    LOCK(cs);

    if (!pas.IsEnabled())
        return false;

    CPubkeyaliasservice *ppas = Find(pas.vin);

    if (ppas == NULL)
    {
        LogPrint("pubkeyaliasservice", "CPubkeyaliasserviceMan: Adding new pubkeyaliasservice entry: %s | Net Total: %i \n", pas.pubkey.GetID().ToString().c_str(), size() + 1);
        vPubkeyaliasservices.push_back(pas);
        return true;
    }

    return false;
}

void CPubkeyaliasserviceMan::AskForPAS(CNode* pnode, CTxIn &vin)
{
    std::map<COutPoint, int64_t>::iterator i = mWeAskedForPubkeyaliasserviceListEntry.find(vin.prevout);
    if (i != mWeAskedForPubkeyaliasserviceListEntry.end())
    {
        int64_t t = (*i).second;
        if (GetTime() < t) return; // we've asked recently
    }

    // ask for the pasb info once from the node that sent pasp

    LogPrintf("CPubkeyaliasserviceMan::AskForPAS - Asking node for missing entry, vin: %s\n", vin.ToString());
    pnode->PushMessage("pasg", vin);
    int64_t askAgain = GetTime() + PUBKEYALIASSERVICES_QUEUE_SECONDS;
    mWeAskedForPubkeyaliasserviceListEntry[vin.prevout] = askAgain;
}

void CPubkeyaliasserviceMan::CheckPAS()
{
    LOCK(cs);

    BOOST_FOREACH(CPubkeyaliasservice& pas, vPubkeyaliasservices)
        pas.Check();
}

void CPubkeyaliasserviceMan::CheckAndRemovePAS()
{
    LOCK(cs);

    CheckPAS();

    //remove inactive
    vector<CPubkeyaliasservice>::iterator it = vPubkeyaliasservices.begin();
    while(it != vPubkeyaliasservices.end()){
        if((*it).activeStatePAS == CPubkeyaliasservice::PUBKEYALIASSERVICE_REMOVE || (*it).activeStatePAS == CPubkeyaliasservice::PUBKEYALIASSERVICE_VIN_ERROR || (*it).protocolVersion < nPubkeyaliasserviceMinProtocol){
            LogPrint("pubkeyaliasservice", "CPubkeyaliasserviceMan: Removing invalid pubkeyaliasservice entry: %s | Net Total: %i \n", (*it).pubkey.GetID().ToString().c_str(), size() - 1);
            it = vPubkeyaliasservices.erase(it);
        } else {
            ++it;
        }
    }

    // check who's asked for the pubkeyaliasservice list
    map<CNetAddr, int64_t>::iterator it1 = mAskedUsForPubkeyaliasserviceList.begin();
    while(it1 != mAskedUsForPubkeyaliasserviceList.end()){
        if((*it1).second < GetTime()) {
            mAskedUsForPubkeyaliasserviceList.erase(it1++);
        } else {
            ++it1;
        }
    }

    // check who we asked for the pubkeyaliasservice list
    it1 = mWeAskedForPubkeyaliasserviceList.begin();
    while(it1 != mWeAskedForPubkeyaliasserviceList.end()){
        if((*it1).second < GetTime()){
            mWeAskedForPubkeyaliasserviceList.erase(it1++);
        } else {
            ++it1;
        }
    }

    // check which pubkeyaliasservices we've asked for
    map<COutPoint, int64_t>::iterator it2 = mWeAskedForPubkeyaliasserviceListEntry.begin();
    while(it2 != mWeAskedForPubkeyaliasserviceListEntry.end()){
        if((*it2).second < GetTime()){
            mWeAskedForPubkeyaliasserviceListEntry.erase(it2++);
        } else {
            ++it2;
        }
    }

}

void CPubkeyaliasserviceMan::ClearPAS()
{
    LOCK(cs);
    vPubkeyaliasservices.clear();
    mAskedUsForPubkeyaliasserviceList.clear();
    mWeAskedForPubkeyaliasserviceList.clear();
    mWeAskedForPubkeyaliasserviceListEntry.clear();
}

int CPubkeyaliasserviceMan::CountEnabledPAS()
{
    int i = 0;

    BOOST_FOREACH(CPubkeyaliasservice& pas, vPubkeyaliasservices) {
        pas.Check();
        if((pas.regTime + PUBKEYALIASSERVICE_EXPIRATION_SECONDS) < GetTime()) continue;
        i++;
    }

    return i;
}

int CPubkeyaliasserviceMan::CountPubkeyaliasservicesAboveProtocolPAS(int minprotocolVersion)
{
    int i = 0;

    BOOST_FOREACH(CPubkeyaliasservice& pas, vPubkeyaliasservices) {
        pas.Check();
        if(pas.protocolVersion < minprotocolVersion || !pas.IsEnabled()) continue;
        i++;
    }

    return i;
}

CPubkeyaliasservice *CPubkeyaliasserviceMan::Find(const CTxIn &vin)
{
    LOCK(cs);

    BOOST_FOREACH(CPubkeyaliasservice& pas, vPubkeyaliasservices)
    {
        if(pas.vin.prevout == vin.prevout)
            return &pas;
    }
    return NULL;
}

CPubkeyaliasservice *CPubkeyaliasserviceMan::Find(const CPubKey &pubKeyPubkeyaliasservice)
{
    LOCK(cs);

    BOOST_FOREACH(CPubkeyaliasservice& pas, vPubkeyaliasservices)
    {
        if(pas.pubkey == pubKeyPubkeyaliasservice)
            return &pas;
    }
    return NULL;
}

CPubkeyaliasservice* CPubkeyaliasserviceMan::FindOldestNotInVec(const std::vector<CTxIn> &vVins, int nMinimumAge)
{
    LOCK(cs);

    CPubkeyaliasservice *pOldestPubkeyaliasservice = NULL;

    BOOST_FOREACH(CPubkeyaliasservice &pas, vPubkeyaliasservices)
    {   
        pas.Check();
        if(!pas.IsEnabled()) continue;

        if(pas.GetPubkeyaliasserviceInputAge() < nMinimumAge) continue;

        bool found = false;
        BOOST_FOREACH(const CTxIn& vin, vVins)
            if(pas.vin.prevout == vin.prevout)
            {   
                found = true;
                break;
            }
        if(found) continue;

        if(pOldestPubkeyaliasservice == NULL || pOldestPubkeyaliasservice->regTime < pas.regTime)
        {
            pOldestPubkeyaliasservice = &pas;
        }
    }

    return pOldestPubkeyaliasservice;
}

void CPubkeyaliasserviceMan::ProcessMessagePAS(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{

    // Ignore relays while we are syncing or if we disabled this feature
    if(!pasEnginePool.IsBlockchainSynced()) return;
    if(!fPubkeyAliasService) return;

    LOCK(cs_process_messagePAS);

    if (strCommand == "pasee") { //MNengine Election Entry

        CTxIn vin;
        CPubKey pubkey;
        int64_t regTime;
        int count;
        int protocolVersion;
        std::string strMessage;

        // 70047 and greater
        vRecv >> vin >> regTime >> pubkey >> count >> protocolVersion;

        // make sure registration time isn't in the future (past is OK)
        if (regTime > GetAdjustedTime() + (1 * 60)) {
            LogPrintf("dsee - Pubkey Alias Service rejected, too far into the future %s\n", vin.ToString().c_str());
            return;
        }

        // TODO: Rework to check if this is our address.
        bool isLocal = false;
        //if() {
            //
        //}

        std::string vchPubKey(pubkey.begin(), pubkey.end());

        strMessage = pubkey.GetID().ToString() + boost::lexical_cast<std::string>(regTime) + vchPubKey + boost::lexical_cast<std::string>(protocolVersion);

        if(protocolVersion < MIN_PASERVICE_PROTO_VERSION) {
            LogPrintf("dsee - ignoring outdated pubkeyaliasservice %s protocol version %d\n", vin.ToString().c_str(), protocolVersion);
            return;
        }

        CScript pubkeyScript;
        pubkeyScript.SetDestination(pubkey.GetID());

        if(pubkeyScript.size() != 25) {
            LogPrintf("dsee - pubkey the wrong size\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        if(!vin.scriptSig.empty()) {
            LogPrintf("dsee - Ignore Not Empty ScriptSig %s\n",vin.ToString().c_str());
            return;
        }

        std::string errorMessage = "";

        //search existing pubkeyaliasservice list, this is where we update existing pubkeyaliasservices with new dsee broadcasts
        CPubkeyaliasservice* ppas = this->Find(vin);
        // if we are a pubkeyaliasservice but with undefined vin and this dsee is ours (matches our Pubkeyaliasservice privkey) then just skip this part
        if(ppas != NULL && !(isLocal))
        {
            // count == -1 when it's a new entry
            //   e.g. We don't want the entry relayed/time updated when we're syncing the list
            // pas.pubkey = pubkey, IsVinAssociatedWithPubkey is validated once below,
            //   after that they just need to match
            if(count == -1 && ppas->pubkey == pubkey){

                if(ppas->regTime < regTime){ //take the newest entry
                    LogPrintf("dsee - Got updated entry for %s\n", pubkey.GetID().ToString().c_str());
                    ppas->protocolVersion = protocolVersion;
                    ppas->Check();
                    if(ppas->IsEnabled())
                        paserviceman.RelayPubkeyaliasserviceEntry(vin, regTime, pubkey, count, protocolVersion);
                }
            }

            return;
        }

        CScript pas_asc_pubkey;
        CPubKey pas_cpubkey;
        pas_asc_pubkey = GetScriptForDestination(CBitcoinAddress(Params().PASfeeAddress()).Get());
        pas_cpubkey = pas_asc_pubkey;

        // make sure the vout/vin that was signed is related to the transaction that spawned the pubkeyaliasservice
        //  - this is expensive, so it's only done once per pubkeyaliasservice
        if(!pasEngineAssociator.IsVinAssociatedWithPubkey(vin, pas_cpubkey)) {
            LogPrintf("dsee - WARNING - Got mismatched pubkey and vin\n");
            //Misbehaving(pfrom->GetId(), 100);
            return;
        }
        if(!pasEngineAssociator.IsVoutAssociatedWithPubkey(vin, pubkey)){
            LogPrintf("dsee - WARNING - Got mismatched pubkey and vout \n");
            //Misbehaving(pfrom->GetId(), 100);
            return;
        }

        LogPrint("pubkeyaliasservice", "dsee - Got NEW pubkeyaliasservice entry %s\n", pubkey.GetID().ToString().c_str());

        // make sure it's still unspent
        //  - this is checked later by .check() in many places and by ThreadCheckMNenginePool()

        CValidationState state;
        CTransaction txVin_man;
        uint256 hash_man;
        bool fAcceptable = false;
        {
            TRY_LOCK(cs_main, lockMain);
            if(!lockMain) return;
            fAcceptable = GetTransaction(vin.prevout.hash, txVin_man, hash_man);
        }
        if(fAcceptable){
            LogPrint("pubkeyaliasservice", "dsee - Accepted pubkeyaliasservice entry %i\n", count);

            if(GetInputAge(vin) < PUBKEYALIASSERVICE_MIN_CONFIRMATIONS){
                LogPrintf("dsee - Input must have least %d confirmations\n", PUBKEYALIASSERVICE_MIN_CONFIRMATIONS);
                //Misbehaving(pfrom->GetId(), 20);
                return;
            }

            // verify that sig time is legit in past
            // should be at least not earlier than block when 75 FrogCoin tx got PUBKEYALIASSERVICE_MIN_CONFIRMATIONS
            uint256 hashBlock = 0;
            GetTransaction(vin.prevout.hash, txVin_man, hashBlock);
            map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
           if (mi != mapBlockIndex.end() && (*mi).second)
            {
                CBlockIndex* pPASIndex = (*mi).second; // block for 75 FrogCoin tx -> 1 confirmation
                CBlockIndex* pConfIndex = FindBlockByHeight((pPASIndex->nHeight + PUBKEYALIASSERVICE_MIN_CONFIRMATIONS - 1)); // block where tx got PUBKEYALIASSERVICE_MIN_CONFIRMATIONS
                if(pConfIndex->GetBlockTime() > regTime)
                {
                    LogPrintf("dsee - Bad regTime %d for pubkeyaliasservice %s %105s (%i conf block is at %d)\n",
                              regTime, pubkey.GetID().ToString(), vin.ToString(), PUBKEYALIASSERVICE_MIN_CONFIRMATIONS, pConfIndex->GetBlockTime());
                    return;
                }
            }

            // add our pubkeyaliasservice
            CPubkeyaliasservice pas(vin, pubkey, regTime, protocolVersion);
            this->AddPAS(pas);

            if(count == -1 && !isLocal)
                paserviceman.RelayPubkeyaliasserviceEntry(vin, regTime, pubkey, count, protocolVersion);

        } else {
            LogPrintf("dsee - Rejected pubkeyaliasservice entry %s\n", pubkey.GetID().ToString().c_str());
            //Misbehaving(pfrom->GetId(), 100);
        }
    }

    else if (strCommand == "paseep") { //MNengine Election Entry Ping

        CTxIn vin;
        int64_t regTime;
        bool stop;
        vRecv >> vin >> regTime >> stop;

        //LogPrintf("dseep - Received: vin: %s sigTime: %lld stop: %s\n", vin.ToString().c_str(), sigTime, stop ? "true" : "false");

        if (regTime > GetAdjustedTime() + (1 * 60)) {
            LogPrintf("dseep - Signature rejected, too far into the future %s\n", vin.ToString().c_str());
            return;
        }

        if (regTime <= GetAdjustedTime() - PUBKEYALIASSERVICE_EXPIRATION_SECONDS) {
            LogPrintf("dseep - Signature rejected, too far into the past %s - %d %d \n", vin.ToString().c_str(), regTime, GetAdjustedTime());
            return;
        }

        // see if we have this pubkeyaliasservice
        CPubkeyaliasservice* ppas = this->Find(vin);
        if(ppas != NULL && ppas->protocolVersion >= MIN_PASERVICE_PROTO_VERSION)
        {
            LogPrintf("dseep - Found corresponding pas for vin: %s\n", vin.ToString().c_str());
            // check our entries only if we haven't in a while
            if(1 == 1)// TODO: toggle this better
            {
                std::string errorMessage = "";
                if(!pasEngineAssociator.IsVoutAssociatedWithPubkey(ppas->vin, ppas->pubkey))
                {
                    LogPrintf("dseep - WARNING - Could not verify pubkeyaliasservice address signature %s \n", vin.ToString().c_str());
                    //Misbehaving(pfrom->GetId(), 100);
                    return;
                }

                ppas->Check();
                if(!ppas->IsEnabled()) return;
                paserviceman.RelayPubkeyaliasserviceEntryPing(vin, regTime, stop);
            }
            return;
        }

        LogPrint("pubkeyaliasservice", "dseep - Couldn't find pubkeyaliasservice entry %s\n", vin.ToString().c_str());

        std::map<COutPoint, int64_t>::iterator i = mWeAskedForPubkeyaliasserviceListEntry.find(vin.prevout);
        if (i != mWeAskedForPubkeyaliasserviceListEntry.end())
        {
            int64_t t = (*i).second;
            if (GetTime() < t) return; // we've asked recently
        }

        // ask for the dsee info once from the node that sent dseep

        LogPrintf("dseep - Asking source node for missing entry %s\n", vin.ToString().c_str());
        pfrom->PushMessage("pasg", vin);
        int64_t askAgain = GetTime()+ PUBKEYALIASSERVICES_QUEUE_SECONDS;
        mWeAskedForPubkeyaliasserviceListEntry[vin.prevout] = askAgain;

    } else if (strCommand == "pasg") { //Get pubkeyaliasservice list or specific entry

        CTxIn vin;
        vRecv >> vin;

        // TODO: Rework to check if this is our address.
        bool isLocal = false;
        //if() {
            //
        //}

        if(vin == CTxIn()) { //only should ask for this once
            //local network
            if(isLocal && Params().NetworkID() == CChainParams::MAIN)
            {
                std::map<CNetAddr, int64_t>::iterator i = mAskedUsForPubkeyaliasserviceList.find(pfrom->addr);
                if (i != mAskedUsForPubkeyaliasserviceList.end())
                {
                    int64_t t = (*i).second;
                    if (GetTime() < t) {
                        Misbehaving(pfrom->GetId(), 34);
                        LogPrintf("dseg - peer already asked me for the list\n");
                        return;
                    }
                }

                int64_t askAgain = GetTime() + PUBKEYALIASSERVICES_QUEUE_SECONDS;
                mAskedUsForPubkeyaliasserviceList[pfrom->addr] = askAgain;
            }
        } //else, asking for a specific alias which is ok

        int count = this->size();
        int i = 0;

        BOOST_FOREACH(CPubkeyaliasservice& pas, vPubkeyaliasservices) {

            if(isLocal) continue; //local network

            if(pas.IsEnabled())
            {
                LogPrint("pubkeyaliasservice", "dseg - Sending pubkeyaliasservice entry - %s \n", pas.pubkey.GetID().ToString().c_str());
                if(vin == CTxIn()){
                    pfrom->PushMessage("pasee", pas.vin, pas.pubkey, count, i, pas.regTime, pas.protocolVersion);
                } else if (vin == pas.vin) {
                    pfrom->PushMessage("pasee", pas.vin, pas.pubkey, count, i, pas.regTime, pas.protocolVersion);
                    LogPrintf("dseg - Sent 1 pubkeyaliasservice entries to %s\n", pfrom->addr.ToString().c_str());
                    return;
                }
                i++;
            }
        }

        LogPrintf("dseg - Sent %d pubkeyaliasservice entries to %s\n", i, pfrom->addr.ToString().c_str());
    }

}

void CPubkeyaliasserviceMan::RelayPubkeyaliasserviceEntry(const CTxIn vin, const int64_t nregTime, const CPubKey pubkey, const int count, const int protocolVersion)
{
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
        pnode->PushMessage("pasee", vin, nregTime, pubkey, count, protocolVersion);
}

void CPubkeyaliasserviceMan::RelayPubkeyaliasserviceEntryPing(const CTxIn vin, const int64_t nregTime, const bool stop)
{
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
        pnode->PushMessage("paseep", vin, nregTime, stop);
}

void CPubkeyaliasserviceMan::RemovePAS(CTxIn vin)
{
    LOCK(cs);

    vector<CPubkeyaliasservice>::iterator it = vPubkeyaliasservices.begin();
    while(it != vPubkeyaliasservices.end()){
        if((*it).vin == vin){
            if((*it).vin.prevPubKey == vin.prevPubKey) {
                LogPrint("pubkeyaliasservice", "CPubkeyaliasserviceMan: Removing Pubkeyaliasservice %s - %i now\n", (*it).vin.prevPubKey.ToString().c_str(), size() - 1);
                vPubkeyaliasservices.erase(it);
                break;
            }
        }
    }
}

std::string CPubkeyaliasserviceMan::ToString() const
{
    std::ostringstream info;

    info << "pubkeyaliasservices: " << (int)vPubkeyaliasservices.size() <<
            ", peers who asked us for pubkeyaliasservice list: " << (int)mAskedUsForPubkeyaliasserviceList.size() <<
            ", peers we asked for pubkeyaliasservice list: " << (int)mWeAskedForPubkeyaliasserviceList.size() <<
            ", entries in Pubkeyaliasservice list we asked for: " << (int)mWeAskedForPubkeyaliasserviceListEntry.size();

    return info.str();
}
