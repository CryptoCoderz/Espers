// Copyright (c) 2009-2012 The Darkcoin developers
// Copyright (c) 2017-2018 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xnodestart.h"
#include "xnodemngr.h"
#include "protocol.h"
#include <boost/lexical_cast.hpp>
#include "clientversion.h"

//
// Bootup the xnode, look for a 5,000 ESP input and register on the network
//
void CActiveXNode::ManageStatus()
{
    std::string errorMessage;

    if (fDebug) LogPrintf("CActiveXNode::ManageStatus() - Begin\n");

    if(!fXnode) return;

    //need correct adjusted time to send ping
    bool fIsInitialDownload = IsInitialBlockDownload();
    if(fIsInitialDownload) {
        status = XNODE_SYNC_IN_PROCESS;
        LogPrintf("CActiveXNode::ManageStatus() - Sync in progress. Must wait until sync is complete to start xnode.\n");
        return;
    }

    if(status == XNODE_INPUT_TOO_NEW || status == XNODE_NOT_CAPABLE || status == XNODE_SYNC_IN_PROCESS){
        status = XNODE_NOT_PROCESSED;
    }

    if(status == XNODE_NOT_PROCESSED) {
        if(strXnodeAddr.empty()) {
            if(!GetLocal(service)) {
                notCapableReason = "Can't detect external address. Please use the xnodeaddr configuration option.";
                status = XNODE_NOT_CAPABLE;
                LogPrintf("CActiveXNode::ManageStatus() - not capable: %s\n", notCapableReason.c_str());
                return;
            }
        } else {
        	service = CService(strXnodeAddr, true);
        }

        LogPrintf("CActiveXNode::ManageStatus() - Checking inbound connection to '%s'\n", service.ToString().c_str());

                  
            if(!ConnectNode((CAddress)service, service.ToString().c_str())){
                notCapableReason = "Could not connect to " + service.ToString();
                status = XNODE_NOT_CAPABLE;
                LogPrintf("CActiveXNode::ManageStatus() - not capable: %s\n", notCapableReason.c_str());
                return;
            }
        

        if(pwalletMain->IsLocked()){
            notCapableReason = "Wallet is locked.";
            status = XNODE_NOT_CAPABLE;
            LogPrintf("CActiveXNode::ManageStatus() - not capable: %s\n", notCapableReason.c_str());
            return;
        }

        // Set defaults
        status = XNODE_NOT_CAPABLE;
        notCapableReason = "Unknown. Check debug.log for more information.\n";

        // Choose coins to use
        CPubKey pubKeyCollateralAddress;
        CKey keyCollateralAddress;
/*
        if(GetXnodeVin(vin, pubKeyCollateralAddress, keyCollateralAddress)) {

            if(GetInputAge(vin) < XNODE_MIN_CONFIRMATIONS){
                notCapableReason = "Input must have least " + boost::lexical_cast<string>(XNODE_MIN_CONFIRMATIONS) +
                        " confirmations - " + boost::lexical_cast<string>(GetInputAge(vin)) + " confirmations";
                LogPrintf("CActiveXNode::ManageStatus() - %s\n", notCapableReason.c_str());
                status = XNODE_INPUT_TOO_NEW;
                return;
            }
*/
            LogPrintf("CActiveXNode::ManageStatus() - Is capable xnode!\n");

            status = XNODE_IS_CAPABLE;
            notCapableReason = "";

            //pwalletMain->LockCoin(vin.prevout);

            // send to all nodes
            CPubKey pubKeyXNode;
            CKey keyXNode;

            if(!xnodeEngineSigner.SetKey(strXnodePrivKey, errorMessage, keyXNode, pubKeyXNode))
            {
                LogPrintf("ActiveXNode::Dseep() - Error upon calling SetKey: %s\n", errorMessage.c_str());
                return;
            }

            /* donations are not supported in Espers.conf */
            CScript donationAddress = CScript();
            int donationPercentage = 0;

            if(!Register(vin, service, keyCollateralAddress, pubKeyCollateralAddress, keyXNode, pubKeyXNode, donationAddress, donationPercentage, errorMessage)) {
                LogPrintf("CActiveXNode::ManageStatus() - Error on Register: %s\n", errorMessage.c_str());
            }

            return;
       // } else {
       //     notCapableReason = "Could not find suitable coins!";
       // 	LogPrintf("CActiveXNode::ManageStatus() - Could not find suitable coins!\n");
       // }
    }

    //send to all peers
    if(!Dseep(errorMessage)) {
    	LogPrintf("CActiveXNode::ManageStatus() - Error on Ping: %s\n", errorMessage.c_str());
    }
}

// Send stop dseep to network for remote xnode
bool CActiveXNode::StopXnode(std::string strService, std::string strKeyXNode, std::string& errorMessage) {
	CTxIn vin;
    CKey keyXNode;
    CPubKey pubKeyXNode;

    if(!xnodeEngineSigner.SetKey(strKeyXNode, errorMessage, keyXNode, pubKeyXNode)) {
        LogPrintf("CActiveXNode::StopXnode() - Error: %s\n", errorMessage.c_str());
        return false;
    }

	return StopXnode(vin, CService(strService, true), keyXNode, pubKeyXNode, errorMessage);
}

// Send stop dseep to network for main xnode
bool CActiveXNode::StopXnode(std::string& errorMessage) {
	if(status != XNODE_IS_CAPABLE && status != XNODE_REMOTELY_ENABLED) {
		errorMessage = "xnode is not in a running status";
    	LogPrintf("CActiveXNode::StopXnode() - Error: %s\n", errorMessage.c_str());
		return false;
	}

	status = XNODE_STOPPED;

    CPubKey pubKeyXNode;
    CKey keyXNode;

    if(!xnodeEngineSigner.SetKey(strXnodePrivKey, errorMessage, keyXNode, pubKeyXNode))
    {
        LogPrintf("Register::ManageStatus() - Error upon calling SetKey: %s\n", errorMessage.c_str());
        return false;
    }

	return StopXnode(vin, service, keyXNode, pubKeyXNode, errorMessage);
}

// Send stop dseep to network for any xnode
bool CActiveXNode::StopXnode(CTxIn vin, CService service, CKey keyXNode, CPubKey pubKeyXNode, std::string& errorMessage) {
    //pwalletMain->UnlockCoin(vin.prevout);
	return Dseep(vin, service, keyXNode, pubKeyXNode, errorMessage, true);
}

bool CActiveXNode::Dseep(std::string& errorMessage) {
	if(status != XNODE_IS_CAPABLE && status != XNODE_REMOTELY_ENABLED) {
		errorMessage = "xnode is not in a running status";
    	LogPrintf("CActiveXNode::Dseep() - Error: %s\n", errorMessage.c_str());
		return false;
	}

    CPubKey pubKeyXNode;
    CKey keyXNode;

    if(!xnodeEngineSigner.SetKey(strXnodePrivKey, errorMessage, keyXNode, pubKeyXNode))
    {
        LogPrintf("CActiveXNode::Dseep() - Error upon calling SetKey: %s\n", errorMessage.c_str());
        return false;
    }

	return Dseep(vin, service, keyXNode, pubKeyXNode, errorMessage, false);
}

bool CActiveXNode::Dseep(CTxIn vin, CService service, CKey keyXNode, CPubKey pubKeyXNode, std::string &retErrorMessage, bool stop) {
    std::string errorMessage;
    std::vector<unsigned char> vchXnodeSignature;
    //std::string strXnodeSignMessage;
    int64_t xNodeSignatureTime = GetAdjustedTime();

    std::string strMessage = service.ToString() + boost::lexical_cast<std::string>(xNodeSignatureTime) + boost::lexical_cast<std::string>(stop);

    if(!xnodeEngineSigner.SignMessage(strMessage, errorMessage, vchXnodeSignature, keyXNode)) {
        //retErrorMessage = "sign message failed: " + errorMessage;
        //LogPrintf("CActiveXNode::Dseep() - Error: %s\n", retErrorMessage.c_str());
        //return false;
        LogPrintf("CActiveXNode::Dseep() - Warning: SignMessage bypassed\n");
    }

    if(!xnodeEngineSigner.VerifyMessage(pubKeyXNode, vchXnodeSignature, strMessage, errorMessage)) {
        //retErrorMessage = "Verify message failed: " + errorMessage;
        //LogPrintf("CActiveXNode::Dseep() - Error: %s\n", retErrorMessage.c_str());
        //return false;
        LogPrintf("CActiveXNode::Dseep() - Warning: VerifyMessage bypassed\n");
    }

    // Update Last Seen timestamp in xnode list
    CXNode* pxn = xnodeman.Find(vin);
    if(pxn != NULL)
    {
        if(stop)
            xnodeman.Remove(pxn->vin);
        else
            pxn->UpdateLastSeen();
    } else {
    	// Seems like we are trying to send a ping while the xnode is not registered in the network
    	retErrorMessage = "XnodeEngine XNode List doesn't include our xnode, Shutting down xnode pinging service! " + vin.ToString();
    	LogPrintf("CActiveXNode::Dseep() - Error: %s\n", retErrorMessage.c_str());
        status = XNODE_NOT_CAPABLE;
        notCapableReason = retErrorMessage;
        return false;
    }

    //send to all peers
    LogPrintf("CActiveXNode::Dseep() - RelayXNodeEntryPing vin = %s\n", vin.ToString().c_str());
    xnodeman.RelayXNodeEntryPing(vin, vchXnodeSignature, xNodeSignatureTime, stop);

    return true;
}

bool CActiveXNode::Register(std::string strService, std::string strKeyXNode, std::string txHash, std::string strOutputIndex, std::string strDonationAddress, std::string strDonationPercentage, std::string& errorMessage) {
    CTxIn vin;
    CPubKey pubKeyCollateralAddress;
    CKey keyCollateralAddress;
    CPubKey pubKeyXNode;
    CKey keyXNode;
    CScript donationAddress = CScript();
    int donationPercentage = 0;

    if(!xnodeEngineSigner.SetKey(strKeyXNode, errorMessage, keyXNode, pubKeyXNode))
    {
        LogPrintf("CActiveXNode::Register() - Error upon calling SetKey: %s\n", errorMessage.c_str());
        return false;
    }

    if(!GetXnodeVin(vin, pubKeyCollateralAddress, keyCollateralAddress, txHash, strOutputIndex)) {
        errorMessage = "could not allocate vin";
        LogPrintf("CActiveXNode::Register() - Error: %s\n", errorMessage.c_str());
        return false;
    }
    //CXNodeAddress address;
    CBitcoinAddress address;
    if (strDonationAddress != "")
    {
        if(!address.SetString(strDonationAddress))
        {
            LogPrintf("ActiveXNode::Register - Invalid Donation Address\n");
            return false;
        }
        donationAddress.SetDestination(address.Get());

        try {
            donationPercentage = boost::lexical_cast<int>( strDonationPercentage );
        } catch( boost::bad_lexical_cast const& ) {
            LogPrintf("ActiveXNode::Register - Invalid Donation Percentage (Couldn't cast)\n");
            return false;
        }

        if(donationPercentage < 0 || donationPercentage > 100)
        {
            LogPrintf("ActiveXNode::Register - Donation Percentage Out Of Range\n");
            return false;
        }
    }

	return Register(vin, CService(strService, true), keyCollateralAddress, pubKeyCollateralAddress, keyXNode, pubKeyXNode, donationAddress, donationPercentage, errorMessage);
}

bool CActiveXNode::Register(CTxIn vin, CService service, CKey keyCollateralAddress, CPubKey pubKeyCollateralAddress, CKey keyXNode, CPubKey pubKeyXNode, CScript donationAddress, int donationPercentage, std::string &retErrorMessage) {
    std::string errorMessage;
    std::vector<unsigned char> vchXnodeSignature;
    //std::string strXnodeSignMessage;
    int64_t xNodeSignatureTime = GetAdjustedTime();

    std::string vchPubKey(pubKeyCollateralAddress.begin(), pubKeyCollateralAddress.end());
    std::string vchPubKey2(pubKeyXNode.begin(), pubKeyXNode.end());

    std::string strMessage = service.ToString() + boost::lexical_cast<std::string>(xNodeSignatureTime) + vchPubKey + vchPubKey2 + boost::lexical_cast<std::string>(PROTOCOL_VERSION) + donationAddress.ToString() + boost::lexical_cast<std::string>(donationPercentage);

    if(!xnodeEngineSigner.SignMessage(strMessage, errorMessage, vchXnodeSignature, keyCollateralAddress)) {
        //retErrorMessage = "sign message failed: " + errorMessage;
        //LogPrintf("CActiveXNode::Register() - Error: %s\n", retErrorMessage.c_str());
        //return false;
        LogPrintf("CActiveXNode::Register() - Warning: SignMessage bypassed\n");
    }

    if(!xnodeEngineSigner.VerifyMessage(pubKeyCollateralAddress, vchXnodeSignature, strMessage, errorMessage)) {
        //retErrorMessage = "Verify message failed: " + errorMessage;
        //LogPrintf("CActiveXNode::Register() - Error: %s\n", retErrorMessage.c_str());
        //return false;
        LogPrintf("CActiveXNode::Register() - Warning: VerifyMessage bypassed\n");
    }

    CXNode* pxn = xnodeman.Find(vin);
    if(pxn == NULL)
    {
        LogPrintf("CActiveXNode::Register() - Adding to xnode list service: %s - vin: %s\n", service.ToString().c_str(), vin.ToString().c_str());
        CXNode xn(service, vin, pubKeyCollateralAddress, vchXnodeSignature, xNodeSignatureTime, pubKeyXNode, PROTOCOL_VERSION, donationAddress, donationPercentage);
        xn.UpdateLastSeen(xNodeSignatureTime);
        xnodeman.Add(xn);
    }

    //send to all peers
    LogPrintf("CActiveXNode::Register() - RelayElectionEntry vin = %s\n", vin.ToString().c_str());
    xnodeman.RelayXNodeEntry(vin, service, vchXnodeSignature, xNodeSignatureTime, pubKeyCollateralAddress, pubKeyXNode, -1, -1, xNodeSignatureTime, PROTOCOL_VERSION, donationAddress, donationPercentage);

    return true;
}

bool CActiveXNode::GetXnodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey) {
	return GetXnodeVin(vin, pubkey, secretKey, "", "");
}

bool CActiveXNode::GetXnodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex) {
    CScript pubScript;

    // Find possible candidates
    vector<COutput> possibleCoins = SelectCoinsXNode();
    COutput *selectedOutput;

    // Find the vin
	if(!strTxHash.empty()) {
		// Let's find it
		uint256 txHash(strTxHash);
        int outputIndex = boost::lexical_cast<int>(strOutputIndex);
		bool found = false;
		BOOST_FOREACH(COutput& out, possibleCoins) {
			if(out.tx->GetHash() == txHash && out.i == outputIndex)
			{
				selectedOutput = &out;
				found = true;
				break;
			}
		}
		if(!found) {
			LogPrintf("CActiveXNode::GetXnodeVin - Could not locate valid vin\n");
			return false;
		}
	} else {
		// No output specified,  Select the first one
		if(possibleCoins.size() > 0) {
			selectedOutput = &possibleCoins[0];
		} else {
			LogPrintf("CActiveXNode::GetXnodeVin - Could not locate specified vin from possible list\n");
			return false;
		}
    }

	// At this point we have a selected output, retrieve the associated info
	return GetVinFromOutput(*selectedOutput, vin, pubkey, secretKey);
}

bool CActiveXNode::GetXnodeVinForPubKey(std::string collateralAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey) {
	return GetXnodeVinForPubKey(collateralAddress, vin, pubkey, secretKey, "", "");
}

bool CActiveXNode::GetXnodeVinForPubKey(std::string collateralAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex) {
    CScript pubScript;

    // Find possible candidates
    vector<COutput> possibleCoins = SelectCoinsXNodeForPubKey(collateralAddress);
    COutput *selectedOutput;

    // Find the vin
	if(!strTxHash.empty()) {
		// Let's find it
		uint256 txHash(strTxHash);
        int outputIndex = boost::lexical_cast<int>(strOutputIndex);
		bool found = false;
		BOOST_FOREACH(COutput& out, possibleCoins) {
			if(out.tx->GetHash() == txHash && out.i == outputIndex)
			{
				selectedOutput = &out;
				found = true;
				break;
			}
		}
		if(!found) {
			LogPrintf("CActiveXNode::GetXnodeVinForPubKey - Could not locate valid vin\n");
			return false;
		}
	} else {
		// No output specified,  Select the first one
		if(possibleCoins.size() > 0) {
			selectedOutput = &possibleCoins[0];
		} else {
			LogPrintf("CActiveXNode::GetXnodeVinForPubKey - Could not locate specified vin from possible list\n");
			return false;
		}
    }

	// At this point we have a selected output, retrieve the associated info
	return GetVinFromOutput(*selectedOutput, vin, pubkey, secretKey);
}


// Extract xnode vin information from output
bool CActiveXNode::GetVinFromOutput(COutput out, CTxIn& vin, CPubKey& pubkey, CKey& secretKey) {

    CScript pubScript;

	vin = CTxIn(out.tx->GetHash(),out.i);
    pubScript = out.tx->vout[out.i].scriptPubKey; // the inputs PubKey

	CTxDestination address1;
    ExtractDestination(pubScript, address1);
    //CXNodeAddress address2(address1);
    CBitcoinAddress address2(address1);

    CKeyID keyID;
    if (!address2.GetKeyID(keyID)) {
        LogPrintf("CActiveXNode::GetXnodeVin - Address does not refer to a key\n");
        return false;
    }

    if (!pwalletMain->GetKey(keyID, secretKey)) {
        LogPrintf ("CActiveXNode::GetXnodeVin - Private key for address is not known\n");
        return false;
    }

    pubkey = secretKey.GetPubKey();
    return true;
}

// get all possible outputs for running xnode
vector<COutput> CActiveXNode::SelectCoinsXNode()
{
    vector<COutput> vCoins;
    vector<COutput> filteredCoins;

    // Retrieve all possible outputs
    pwalletMain->AvailableCoinsXN(vCoins);

    // Filter
    BOOST_FOREACH(const COutput& out, vCoins)
    {
        if(out.tx->vout[out.i].nValue == XNodeCollateral(pindexBest->nHeight)*COIN) { //exactly
        	filteredCoins.push_back(out);
        }
    }
    return filteredCoins;
}

// get all possible outputs for running xnode for a specific pubkey
vector<COutput> CActiveXNode::SelectCoinsXNodeForPubKey(std::string collateralAddress)
{
    //CXNodeAddress address(collateralAddress);
    CBitcoinAddress address(collateralAddress);
    CScript scriptPubKey;
    scriptPubKey.SetDestination(address.Get());
    vector<COutput> vCoins;
    vector<COutput> filteredCoins;

    // Retrieve all possible outputs
    pwalletMain->AvailableCoins(vCoins);

    // Filter
    BOOST_FOREACH(const COutput& out, vCoins)
    {
        if(out.tx->vout[out.i].scriptPubKey == scriptPubKey && out.tx->vout[out.i].nValue == XNodeCollateral(pindexBest->nHeight)*COIN) { //exactly
        	filteredCoins.push_back(out);
        }
    }
    return filteredCoins;
}

// when starting a xnode, this can enable to run as a hot wallet with no funds
bool CActiveXNode::EnableHotColdXnode(CTxIn& newVin, CService& newService)
{
    if(!fXnode) return false;

    status = XNODE_REMOTELY_ENABLED;

    //The values below are needed for signing dseep messages going forward
    this->vin = newVin;
    this->service = newService;

    LogPrintf("CActiveXNode::EnableHotColdXnode() - Enabled! You may shut down the cold daemon.\n");

    return true;
}
