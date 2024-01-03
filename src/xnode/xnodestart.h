// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The DarkCoin developers
// Copyright (c) 2017-2024 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ACTIVEXNODE_H
#define ACTIVEXNODE_H

#include "primitives/uint256.h"
#include "core/sync.h"
#include "node/net.h"
#include "subcore/key.h"
#include "xnodecomponent.h"
#include "xnodecrypter.h"
#include "core/main.h"
#include "util/init.h"
#include "core/wallet.h"

// Responsible for activating the xnode and pinging the network
class CActiveXNode
{
public:
	// Initialized by init.cpp
	// Keys for the main xnode
	CPubKey pubKeyXNode;

	// Initialized while registering xnode
	CTxIn vin;
    CService service;

    int status;
    std::string notCapableReason;

    CActiveXNode()
    {        
        status = XNODE_NOT_PROCESSED;
    }

    void ManageStatus(); // manage status of main xnode

    bool Dseep(std::string& errorMessage); // ping for main xnode
    bool Dseep(CTxIn vin, CService service, CKey key, CPubKey pubKey, std::string &retErrorMessage, bool stop); // ping for any xnode

    bool StopXnode(std::string& errorMessage); // stop main xnode
    bool StopXnode(std::string strService, std::string strKeyXNode, std::string& errorMessage); // stop remote xnode
    bool StopXnode(CTxIn vin, CService service, CKey key, CPubKey pubKey, std::string& errorMessage); // stop any xnode

    /// Register remote XNode
    bool Register(std::string strService, std::string strKey, std::string txHash, std::string strOutputIndex, std::string strDonationAddress, std::string strDonationPercentage, std::string& errorMessage); 
    /// Register any XNode
    bool Register(CTxIn vin, CService service, CKey key, CPubKey pubKey, CKey keyXNode, CPubKey pubKeyXNode, CScript donationAddress, int donationPercentage, std::string &retErrorMessage);  

    // get 5,000 ESP input that can be used for the xnode
    bool GetXnodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey);
    bool GetXnodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex);
    bool GetXnodeVinForPubKey(std::string collateralAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey);
    bool GetXnodeVinForPubKey(std::string collateralAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex);
    vector<COutput> SelectCoinsXNode();
    vector<COutput> SelectCoinsXNodeForPubKey(std::string collateralAddress);
    bool GetVinFromOutput(COutput out, CTxIn& vin, CPubKey& pubkey, CKey& secretKey);

    // enable hot wallet mode (run a xnode with no funds)
    bool EnableHotColdXnode(CTxIn& vin, CService& addr);
};

#endif
