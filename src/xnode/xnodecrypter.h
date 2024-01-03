// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2017-2024 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XNODECRYPTER_H
#define XNODECRYPTER_H

#include "core/chain.h"
#include "core/main.h"
#include "xnodesync.h"
#include "xnodestart.h"
#include "xnodemngr.h"
#include "xnodereward.h"

class CTxIn;
class CXNodeEngineSigner;
class CXnodeVote;
//class CXNodeAddress;
class CActiveXNode;

extern CXNodeEngineSigner xnodeEngineSigner;
extern CActiveXNode activeXNode;
extern std::string strXnodePrivKey;

/** Helper object for signing and checking signatures
 */
class CXNodeEngineSigner
{
public:
    /// Is the inputs associated with this public key? (and there is 5000 ESP - checking if valid xnode)
    bool IsVinAssociatedWithPubkey(CTxIn& vin, CPubKey& pubkey);
    /// Set the private/public key values, returns true if successful
    bool SetKey(std::string strSecret, std::string& errorMessage, CKey& key, CPubKey& pubkey);
    /// Sign the message, returns true if successful
    bool SignMessage(std::string strMessage, std::string& errorMessage, std::vector<unsigned char>& vchSig, CKey key);
    /// Verify the message, returns true if succcessful
    bool VerifyMessage(CPubKey pubkey, std::vector<unsigned char>& vchSig, std::string strMessage, std::string& errorMessage);
};

void ThreadCheckXNodeEnginePool();

#endif // XNODECRYPTER_H
