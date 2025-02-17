// Copyright (c) 2022-2025 The CryptoCoderz Team / Espers project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PASengine_H
#define PASengine_H

#include "core/chain.h"
#include "core/main.h"
#include "core/sync.h"

class CTxIn;
class CFrogCoinAddress;
class CPASenginePool;
class CPASengineAssociator;

extern CPASenginePool pasEnginePool;
extern CPASengineAssociator pasEngineAssociator;

/** Helper object for checking inputs of fees paid
 */
class CPASengineAssociator
{
public:
    /// Is the inputs associated with this public key? (and there is 78 FROG - checking if valid fee)
    bool IsVinAssociatedWithPubkey(CTxIn& vin, CPubKey& pubkey);
    bool IsVoutAssociatedWithPubkey(CTxIn& vin, CPubKey& PASpubkey);
};

/** Used to keep track of current status of PASengine readiness
 */
class CPASenginePool
{
private:
    /// blank

public:
    
    /// from masternode-sync.h
    bool IsBlockchainSynced();

    // where collateral should be made out to
    CScript PASfeePubKey;

};

#endif
