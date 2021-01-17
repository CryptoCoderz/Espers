// Copyright (c) 2016-2021 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ESPERS_FORK_H
#define ESPERS_FORK_H

#include "primitives/bignum.h"

#include "core/sync.h"
#include "node/net.h"
#include "subcore/key.h"
#include "util/util.h"
#include "subcore/script.h"
#include "primitives/base58.h"
#include "primitives/serialize.h"
#include "subcore/protocol.h"
#include <boost/lexical_cast.hpp>

class CSporkMessage;
class CSporkManager;

using namespace std;
using namespace boost;

extern std::map<uint256, CSporkMessage> mapSporks;
extern std::map<int, CSporkMessage> mapSporksActive;
extern CSporkManager sporkManager;

/** Genesis Block Height */
static const int64_t nGenesisHeight = 0;
/** Reserve Phase start block */
static const int64_t nReservePhaseStart = 10;
/** Reserve Phase end block */
static const int64_t nReservePhaseEnd = 356; // rounded +95 blocks for ~17.2 Swap mine
/** Target Blocktime Retard */
static const int64_t nBlocktimeregress = 125000; // Retard block time
/** Espers system patch fork*/
static const int64_t nGravityFork = 615000; // Light Espers chain fork for DarkGravityWave and block time redux.
/** Espers low gravity fix fork*/
static const int64_t nlowGravity = 642000; // Correct low gravity issue with DGW implementation.
/** PoS25 Phase start block */
static const int64_t nPoS25PhaseStart = 20000; // Dropped date due to 25% staking miscalculation
/** PoS5 Phase start block */
static const int64_t nPoS5PhaseStart = 2000800; // Begins @ ~48892586514.24 ESP
/** PoS1 Phase start block */
static const int64_t nPoS1PhaseStart = 3000300; // Begins @ ~48892586514.24 ESP
/** System Upgrade 01 */
static const int64_t sysUpgrade_01 = 674400; // Start swinging difficulty skew, and adaptive block sizes
/** Block type swing patch */
static const int64_t SWING_PATCH = 1509537600; // Patch skew to a more even swing w/ 50/50 block select
/** Value patch on swing */
static const int64_t STALL_PULL = 1515153600; // Revision to swing patch | Friday, January 5, 2018 12:00:00 PM
/** Velocity toggle block */
static const int64_t VELOCITY_TOGGLE = 650000; // Implementation of the Velocity system into the chain.
/** Velocity retarget toggle block */
static const int64_t VELOCITY_TDIFF = 667350; // Use Velocity's retargetting method.
/** Proof-of-Stake Version 3.0 implementation fork */
inline bool IsProtocolV3(int64_t nTime) { return TestNet() || nTime > 1535673600; } // ON (TOGGLED Fri, 31 Aug 2018 00:00:00 GMT)

// Don't ever reuse these IDs for other sporks
#define SPORK_1_XNODE_PAYMENTS_ENFORCEMENT               10000
#define SPORK_6_REPLAY_BLOCKS                                 10005
#define SPORK_7_XNODE_SCANNING                           10006
#define SPORK_8_XNODE_PAYMENT_ENFORCEMENT                10007
#define SPORK_10_XNODE_PAY_UPDATED_NODES                 10009
#define SPORK_12_RECONSIDER_BLOCKS                            10011

#define SPORK_1_XNODE_PAYMENTS_ENFORCEMENT_DEFAULT       4070908800   // OFF
#define SPORK_4_RECONVERGE_DEFAULT                            0            // ON - BUT NOT USED
#define SPORK_6_REPLAY_BLOCKS_DEFAULT                         0            // ON - BUT NOT USED
#define SPORK_8_XNODE_PAYMENT_ENFORCEMENT_DEFAULT        4070908800   // OFF
#define SPORK_10_XNODE_PAY_UPDATED_NODES_DEFAULT         4070908800   // OFF
#define SPORK_12_RECONSIDER_BLOCKS_DEFAULT                    0            // ON

void ProcessSpork(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);
int64_t GetSporkValue(int nSporkID);
bool IsSporkActive(int nSporkID);
void ExecuteSpork(int nSporkID, int nValue);
void ReprocessBlocks(int nBlocks);

//
// Spork Class
// Keeps track of all of the network spork settings
//

class CSporkMessage
{
public:
    std::vector<unsigned char> vchSig;
    int nSporkID;
    int64_t nValue;
    int64_t nTimeSigned;

    uint256 GetHash(){
        uint256 n = Hash(BEGIN(nSporkID), END(nTimeSigned));
        return n;
    }

    IMPLEMENT_SERIALIZE
    (
            READWRITE(nSporkID);
            READWRITE(nValue);
            READWRITE(nTimeSigned);
            READWRITE(vchSig);
    )
};


class CSporkManager
{
private:
    std::vector<unsigned char> vchSig;

    std::string strMasterPrivKey;
    std::string strTestPubKey;
    std::string strMainPubKey;

public:

    CSporkManager() {
        strMainPubKey = "01a187356a4c6ebbf491443ebfa1207275d71cb009f201c118b00cf8e77641c7f1e63e330ba909842c009af375c0f5c1c7368e8d7e2066168c40ce3cb629cf212f";
        strTestPubKey = "01b758974b1c6ebbf491443ebfa1207275d71cb009f201c118b00cf8e77641c7f1e63e330ba909842c009af375c0f5c1c7368e8d7e2066168c40ce3cb629cf212f";
    }

    std::string GetSporkNameByID(int id);
    int GetSporkIDByName(std::string strName);
    bool UpdateSpork(int nSporkID, int64_t nValue);
    bool SetPrivKey(std::string strPrivKey);
    bool CheckSignature(CSporkMessage& spork);
    bool Sign(CSporkMessage& spork);
    void Relay(CSporkMessage& msg);

};

#endif // ESPERS_FORK_H
