// Copyright (c) 2014-2015 The Darkcoin developers
// Copyright (c) 2017-2021 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xnodecrypter.h"

using namespace std;
using namespace boost;

// A helper object for signing messages from XNodes
CXNodeEngineSigner xnodeEngineSigner;
// Keep track of the active XNode
CActiveXNode activeXNode;

bool CXNodeEngineSigner::IsVinAssociatedWithPubkey(CTxIn& vin, CPubKey& pubkey){
    CScript payee2;
    payee2 = GetScriptForDestination(pubkey.GetID());

    CTransaction txVin;
    uint256 hash;
    //if(GetTransaction(vin.prevout.hash, txVin, hash, true)){
    if(GetTransaction(vin.prevout.hash, txVin, hash)){
        BOOST_FOREACH(CTxOut out, txVin.vout){
            if(out.nValue == XNodeCollateral(pindexBest->nHeight)*COIN){
                if(out.scriptPubKey == payee2) return true;
            }
        }
    }

    return false;
}

bool CXNodeEngineSigner::SetKey(std::string strSecret, std::string& errorMessage, CKey& key, CPubKey& pubkey){
    //CXNodeSecret vchSecret;
    CBitcoinSecret vchSecret;
    bool fGood = vchSecret.SetString(strSecret);

    if (!fGood) {
        errorMessage = _("Invalid private key.");
        return false;
    }

    key = vchSecret.GetKey();
    pubkey = key.GetPubKey();

    return true;
}

bool CXNodeEngineSigner::SignMessage(std::string strMessage, std::string& errorMessage, vector<unsigned char>& vchSig, CKey key)
{
    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    if (!key.SignCompact(ss.GetHash(), vchSig)) {
        errorMessage = _("Signing failed.");
        return false;
    }

    return true;
}

bool CXNodeEngineSigner::VerifyMessage(CPubKey pubkey, vector<unsigned char>& vchSig, std::string strMessage, std::string& errorMessage)
{
    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    CPubKey pubkey2;
    if (!pubkey2.RecoverCompact(ss.GetHash(), vchSig)) {
        errorMessage = _("Error recovering public key.");
        return false;
    }

    if (fDebug && (pubkey2.GetID() != pubkey.GetID()))
        LogPrintf("CXNodeEngineSigner::VerifyMessage -- keys don't match: %s %s\n", pubkey2.GetID().ToString(), pubkey.GetID().ToString());

    return (pubkey2.GetID() == pubkey.GetID());
}

void ThreadCheckXNodeEnginePool()
{

    // Make this thread recognisable as the wallet flushing thread
    RenameThread("Espers-xnodeengine");

    unsigned int c = 0;
    unsigned int r = 0;

    while (true)
    {
        MilliSleep(1000);
        //LogPrintf("ThreadCheckXNodeEnginePool::check timeout\n");

        // try to sync from all available nodes, one step at a time
        // TODO: Re-enable later after feature is finished
        //xnodeSync.Process();

        if(xnodeSync.IsXNodeSynced()) {

            c++;

            // check if we should activate or ping every few minutes,
            // start right after sync is considered to be done
            if(c % XNODE_PING_SECONDS == 1) activeXNode.ManageStatus();

            if(c % 60 == 0)
            {
                xnodeman.CheckAndRemove();
                xnodeman.ProcessXNodeConnections();
                xnodePayments.CleanPaymentList();
            }

            //if(c % XNODES_DUMP_SECONDS == 0) DumpXNodes();

        } else {
            if(r == 0) {
                LogPrintf("ThreadCheckXNodeEnginePool - xnodeSync failed!\n");
            }
            // Log runs
            r ++;
        }
    }
}
