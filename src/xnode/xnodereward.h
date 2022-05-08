// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2017-2022 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XNODE_PAYMENTS_H
#define XNODE_PAYMENTS_H

#include "core/sync.h"
#include "node/net.h"
#include "subcore/key.h"
#include "util/util.h"
#include "primitives/base58.h"
#include "core/main.h"
#include "xnodecomponent.h"

using namespace std;

class CXNodePayments;
class CXNodePaymentWinner;

extern CXNodePayments xnodePayments;
extern map<uint256, CXNodePaymentWinner> mapSeenXNodeVotes;

void ProcessMessageXNodePayments(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);
void DumpXNodePayments();

/** Save XNode Payment Data (xnoderewards.dat)
 */
class CXNodePaymentDB
{
private:
    boost::filesystem::path pathDB;
    std::string strMagicMessage;
public:
    enum ReadResult {
        Ok,
        FileError,
        HashReadError,
        IncorrectHash,
        IncorrectMagicMessage,
        IncorrectMagicNumber,
        IncorrectFormat
    };

    CXNodePaymentDB();
    bool Write(const CXNodePayments &objToSave);
    ReadResult Read(CXNodePayments& objToLoad);
};

// for storing the winning payments
class CXNodePaymentWinner
{
public:
    int nBlockHeight;
    CTxIn vin;
    CScript payee;
    std::vector<unsigned char> vchSig;
    uint64_t score;

    CXNodePaymentWinner() {
        nBlockHeight = 0;
        score = 0;
        vin = CTxIn();
        payee = CScript();
    }

    uint256 GetHash(){
        uint256 n2 = Hash(BEGIN(nBlockHeight), END(nBlockHeight));
        uint256 n3 = vin.prevout.hash > n2 ? (vin.prevout.hash - n2) : (n2 - vin.prevout.hash);

        return n3;
    }

    IMPLEMENT_SERIALIZE(
        READWRITE(nBlockHeight);
        READWRITE(payee);
        READWRITE(vin);
        READWRITE(score);
        READWRITE(vchSig);
    )
};

//
// XNode Payments Class
// Keeps track of who should get paid for which blocks
//

class CXNodePayments
{
private:
    std::vector<CXNodePaymentWinner> vWinning;
    int nSyncedFromPeer;
    std::string strMasterPrivKey;
    std::string strMainPubKey;
    bool enabled;
    int nLastBlockHeight;

public:

    CXNodePayments() {
        strMainPubKey = "";
        enabled = false;
    }

    bool SetPrivKey(std::string strPrivKey);
    bool CheckSignature(CXNodePaymentWinner& winner);
    bool Sign(CXNodePaymentWinner& winner);

    // Deterministically calculate a given "score" for a xnode depending on how close it's hash is
    // to the blockHeight. The further away they are the better, the furthest will win the election
    // and get paid this block
    //

    uint64_t CalculateScore(uint256 blockHash, CTxIn& vin);
    bool GetWinningXNode(int nBlockHeight, CTxIn& vinOut);
    bool AddWinningXNode(CXNodePaymentWinner& winner);
    bool ProcessBlock(int nBlockHeight);
    void Relay(CXNodePaymentWinner& winner);
    void Sync(CNode* node);
    void CleanPaymentList();
    // Clear payments vector
    void Clear();
    int LastPayment(CXNode& xn);
    int GetMinXNodePaymentsProto();
    std::string ToString() const;
    bool GetBlockPayee(int nBlockHeight, CScript& payee, CTxIn& vin);

    IMPLEMENT_SERIALIZE()
};


#endif
