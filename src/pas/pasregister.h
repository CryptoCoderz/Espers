// Copyright (c) 2022-2025 The CryptoCoderz Team / Espers project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef PASREGISTER_H
#define PASREGISTER_H

#include "primitives/uint256.h"
#include "core/sync.h"
#include "node/net.h"
#include "subcore/key.h"
#include "pas.h"
#include "core/main.h"
#include "util/init.h"
#include "core/wallet.h"
#include "pasengine.h"

// Responsible for activating the paservice and pinging the network
class CPASregister;
extern CPASregister pasReg;

class CPASregister
{
public:
	// Initialized by init.cpp
	// Keys for the main paservice
	CPubKey pubKeyPaservice;

	// Initialized while registering paservice
    CTxIn vin;

    int status;
    std::string notCapableReason;

    CPASregister()
    {        
        status = PUBKEYALIASSERVICE_NOT_PROCESSED;
    }

    void ManageStatus(); // manage status of main paservice

    bool PAseep(std::string& errorMessage); // ping for main paservice
    bool PAseep(CTxIn vin, std::string &retErrorMessage, bool stop); // ping for any paservice

    bool StopPaService(std::string& errorMessage); // stop paservice
    bool StopPaService(CTxIn vin, std::string& errorMessage); // stop any paservice

    /// Register remote Paservice
    bool Register(std::string txHash, std::string strOutputIndex, std::string& errorMessage);
    /// Register any Paservice
    bool Register(CTxIn vin, CPubKey pubKeyFeeAddress, std::string &retErrorMessage);

    // get ESP input that can be used for the paservice
    bool GetPaServiceVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey);
    bool GetPaServiceVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex);
    bool GetPaServiceVinForPubKey(std::string collateralAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey);
    bool GetPaServiceVinForPubKey(std::string collateralAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex);
    vector<COutput> SelectCoinsPaservice();
    vector<COutput> SelectCoinsPaserviceForPubKey(std::string collateralAddress);
    bool GetVinFromOutput(COutput out, CTxIn& vin, CPubKey& pubkey, CKey& secretKey);
};

#endif
