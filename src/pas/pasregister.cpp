// Copyright (c) 2022-2023 The CryptoCoderz Team / Espers project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "subcore/protocol.h"
#include "pasregister.h"
#include "pasman.h"
#include <boost/lexical_cast.hpp>
#include "consensus/clientversion.h"

//
// Bootup the paservice, look for a ESP input and register on the network
//
CPASregister pasReg;

void CPASregister::ManageStatus()
{
    std::string errorMessage;

    if (fDebug) LogPrintf("CPASregister::ManageStatus() - Begin\n");

    //TODO: Only run this if registering...
    if(!fPubkeyAliasService) return;

    //need correct adjusted time to send ping
    bool fIsInitialDownload = IsInitialBlockDownload();
    if(fIsInitialDownload) {
        status = PUBKEYALIASSERVICE_SYNC_IN_PROCESS;
        LogPrintf("CPASregister::ManageStatus() - Sync in progress. Must wait until sync is complete to start paservice.\n");
        return;
    }

    if(status == PUBKEYALIASSERVICE_INPUT_TOO_NEW || status == PUBKEYALIASSERVICE_NOT_CAPABLE || status == PUBKEYALIASSERVICE_SYNC_IN_PROCESS){
        status = PUBKEYALIASSERVICE_NOT_PROCESSED;
    }

    if(status == PUBKEYALIASSERVICE_NOT_PROCESSED) {
        if(pwalletMain->IsLocked()){
            notCapableReason = "Wallet is locked.";
            status = PUBKEYALIASSERVICE_NOT_CAPABLE;
            LogPrintf("CPASregister::ManageStatus() - not capable: %s\n", notCapableReason.c_str());
            return;
        }

        // Set defaults
        status = PUBKEYALIASSERVICE_NOT_CAPABLE;
        notCapableReason = "Unknown. Check debug.log for more information.\n";

        // Choose coins to use
        CPubKey pubKeyFeeAddress;
        CKey keyFeeAddress;

        if(GetPaServiceVin(vin, pubKeyFeeAddress, keyFeeAddress)) {

            if(GetInputAge(vin) < PUBKEYALIASSERVICE_MIN_CONFIRMATIONS){
                notCapableReason = "Input must have least " + boost::lexical_cast<string>(PUBKEYALIASSERVICE_MIN_CONFIRMATIONS) +
                        " confirmations - " + boost::lexical_cast<string>(GetInputAge(vin)) + " confirmations";
                LogPrintf("CPASregister::ManageStatus() - %s\n", notCapableReason.c_str());
                status = PUBKEYALIASSERVICE_INPUT_TOO_NEW;
                return;
            }

            LogPrintf("CPASregister::ManageStatus() - Is capable master node!\n");

            status = PUBKEYALIASSERVICE_IS_CAPABLE;
            notCapableReason = "";

            pwalletMain->LockCoin(vin.prevout);

            // send to all pubkey aliases
            if(!Register(vin, pubKeyFeeAddress, errorMessage)) {
                LogPrintf("CPASregister::ManageStatus() - Error on Register: %s\n", errorMessage.c_str());
            }

            return;
        } else {
            notCapableReason = "Could not find suitable coins!";
            LogPrintf("CPASregister::ManageStatus() - Could not find suitable coins!\n");
        }
    }

    //send to all peers
    if(!PAseep(errorMessage)) {
        LogPrintf("CPASregister::ManageStatus() - Error on Ping: %s\n", errorMessage.c_str());
    }
}

// Send stop paseep to network for main paservice
bool CPASregister::StopPaService(std::string& errorMessage) {
    CKey keyPaservice;
    CPubKey pubKeyPaservice;

    if(status != PUBKEYALIASSERVICE_IS_CAPABLE) {
        errorMessage = "paservice registration is not in a running state";
        LogPrintf("CPASregister::StopPaService() - Service Stop Error(00): %s\n", errorMessage.c_str());
		return false;
	}

    status = PUBKEYALIASSERVICE_STOPPED;

    if (GetPaServiceVin(vin, pubKeyPaservice, keyPaservice)){
        LogPrintf("PaserviceStop::VinFound: %s\n", vin.ToString());
    }

    return StopPaService(vin, errorMessage);
}

// Send stop paseep to network for any paservice
bool CPASregister::StopPaService(CTxIn vin, std::string& errorMessage) {
   	pwalletMain->UnlockCoin(vin.prevout);
    return PAseep(vin, errorMessage, true);
}

bool CPASregister::PAseep(std::string& errorMessage) {
    if(status != PUBKEYALIASSERVICE_IS_CAPABLE) {
        errorMessage = "paservice registration is not in a running status";
        LogPrintf("CPASregister::Dseep() - Service Stop Error(01): %s\n", errorMessage.c_str());
		return false;
    }

    return PAseep(vin, errorMessage, false);
}

bool CPASregister::PAseep(CTxIn vin, std::string &retErrorMessage, bool stop) {
    std::string errorMessage;
    int64_t paServiceRegTime = GetAdjustedTime();

    // TODO: Have proper checks for valid PAS entries
    CPubkeyaliasservice* ppas = paserviceman.Find(vin);
    if(ppas != NULL)
    {
        if(stop) {
            paserviceman.RemovePAS(ppas->vin);
        }
    } else {
    	// Seems like we are trying to send a ping while the paservice is not registered in the network
        retErrorMessage = "PASengine Paservice List doesn't include our registration, Shutting down paservice pinging service! " + vin.ToString();
        LogPrintf("CPASregister::Dseep() - Error: %s\n", retErrorMessage.c_str());
        status = PUBKEYALIASSERVICE_NOT_CAPABLE;
        notCapableReason = retErrorMessage;
        return false;
    }

    //send to all peers
    LogPrintf("CPASregister::Dseep() - RelayPaserviceEntryPing vin = %s\n", vin.ToString().c_str());
    paserviceman.RelayPubkeyaliasserviceEntryPing(vin, paServiceRegTime, stop);

    return true;
}

//TODO: Rewrite this, no need for two internally linked functions...
bool CPASregister::Register(std::string txHash, std::string strOutputIndex, std::string& errorMessage) {
    CTxIn vin;
    CPubKey pubKeyFeeAddress;
    CKey keyFeeAddress;

    if(!GetPaServiceVin(vin, pubKeyFeeAddress, keyFeeAddress, txHash, strOutputIndex)) {
        errorMessage = "could not allocate vin";
        LogPrintf("CPASregister::Register() - Error: %s\n", errorMessage.c_str());
        return false;
    }

    return Register(vin, pubKeyFeeAddress, errorMessage);
}

bool CPASregister::Register(CTxIn vin, CPubKey pubKeyFeeAddress, std::string &retErrorMessage) {
    std::string errorMessage;
    int64_t paServiceRegTime = GetAdjustedTime();

    // TODO: Add aditional checks (if already hosting, deny later regtime)

    CPubkeyaliasservice* ppas = paserviceman.Find(vin);
    if(ppas == NULL)
    {
        //pas(vin, pubkey, regTime, protocolVersion)
        LogPrintf("CPASregister::Register() - Adding to paservice list: %s - vin: %s\n", vin.ToString().c_str());
        CPubkeyaliasservice cpas(vin, pubKeyFeeAddress, paServiceRegTime, PROTOCOL_VERSION);
        paserviceman.AddPAS(cpas);
    } else {
        LogPrintf("CPASregister::Register() - Failed with error: %s - vin: %s\n", retErrorMessage.c_str());
        return false;
    }

    //send to all peers
    //vin, nregTime, pubkey, count, protocolVersion
    LogPrintf("CPASregister::Register() - RelayElectionEntry vin = %s\n", vin.ToString().c_str());
    paserviceman.RelayPubkeyaliasserviceEntry(vin, paServiceRegTime, pubKeyFeeAddress, -1, PROTOCOL_VERSION);

    return true;
}

bool CPASregister::GetPaServiceVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey) {
	return GetPaServiceVin(vin, pubkey, secretKey, "", "");
}

bool CPASregister::GetPaServiceVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex) {
    CScript pubScript;

    // Find possible candidates
    vector<COutput> possibleCoins = SelectCoinsPaservice();
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
            LogPrintf("CPASregister::GetPaServiceVin - Could not locate valid vin\n");
			return false;
		}
	} else {
		// No output specified,  Select the first one
		if(possibleCoins.size() > 0) {
			selectedOutput = &possibleCoins[0];
		} else {
            LogPrintf("CPASregister::GetPaServiceVin - Could not locate specified vin from possible list\n");
			return false;
		}
    }

	// At this point we have a selected output, retrieve the associated info
	return GetVinFromOutput(*selectedOutput, vin, pubkey, secretKey);
}

bool CPASregister::GetPaServiceVinForPubKey(std::string feeAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey) {
    return GetPaServiceVinForPubKey(feeAddress, vin, pubkey, secretKey, "", "");
}

bool CPASregister::GetPaServiceVinForPubKey(std::string feeAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex) {
    CScript pubScript;

    // Find possible candidates
    vector<COutput> possibleCoins = SelectCoinsPaserviceForPubKey(feeAddress);
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
            LogPrintf("CPASregister::GetPaServiceVinForPubKey - Could not locate valid vin\n");
			return false;
		}
	} else {
		// No output specified,  Select the first one
		if(possibleCoins.size() > 0) {
			selectedOutput = &possibleCoins[0];
		} else {
            LogPrintf("CPASregister::GetPaServiceVinForPubKey - Could not locate specified vin from possible list\n");
			return false;
		}
    }

	// At this point we have a selected output, retrieve the associated info
	return GetVinFromOutput(*selectedOutput, vin, pubkey, secretKey);
}


// Extract paservice vin information from output
bool CPASregister::GetVinFromOutput(COutput out, CTxIn& vin, CPubKey& pubkey, CKey& secretKey) {

    CScript pubScript;

	vin = CTxIn(out.tx->GetHash(),out.i);
    pubScript = out.tx->vout[out.i].scriptPubKey; // the inputs PubKey

	CTxDestination address1;
    ExtractDestination(pubScript, address1);
    CBitcoinAddress address2(address1);

    CKeyID keyID;
    if (!address2.GetKeyID(keyID)) {
        LogPrintf("CPASregister::GetPaServiceVin - Address does not refer to a key\n");
        return false;
    }

    if (!pwalletMain->GetKey(keyID, secretKey)) {
        LogPrintf ("CPASregister::GetPaServiceVin - Private key for address is not known\n");
        return false;
    }

    pubkey = secretKey.GetPubKey();
    return true;
}

// get all possible outputs for running paservice
vector<COutput> CPASregister::SelectCoinsPaservice()
{
    vector<COutput> vCoins;
    vector<COutput> filteredCoins;

    // Retrieve all possible outputs
    // TODO: Rewrite, this should refrence PAS not XN
    pwalletMain->AvailableCoinsXN(vCoins);

    // Filter
    BOOST_FOREACH(const COutput& out, vCoins)
    {
        if(out.tx->vout[out.i].nValue == PubkeyaliasserviceFEE(pindexBest->nHeight)*COIN) { //exactly
            filteredCoins.push_back(out);
        }
    }
    return filteredCoins;
}

// get all possible outputs for running paservice for a specific pubkey
vector<COutput> CPASregister::SelectCoinsPaserviceForPubKey(std::string feeAddress)
{
    CBitcoinAddress address(feeAddress);
    CScript scriptPubKey;
    scriptPubKey.SetDestination(address.Get());
    vector<COutput> vCoins;
    vector<COutput> filteredCoins;

    // Retrieve all possible outputs
    pwalletMain->AvailableCoins(vCoins);

    // Filter
    BOOST_FOREACH(const COutput& out, vCoins)
    {
        if(out.tx->vout[out.i].scriptPubKey == scriptPubKey && out.tx->vout[out.i].nValue == PubkeyaliasserviceFEE(pindexBest->nHeight)*COIN) { //exactly
        	filteredCoins.push_back(out);
        }
    }
    return filteredCoins;
}
