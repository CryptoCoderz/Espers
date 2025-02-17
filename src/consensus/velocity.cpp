// Copyright (c) 2014 The Cryptocoin Revival Foundation
// Copyright (c) 2015-2025 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "database/txdb.h"
#include "velocity.h"

bool VELOCITY_FACTOR = false;
uint256 RollingBlock;
int64_t RollingHeight;

/* VelocityI(int nHeight) ? i : -1
   Returns i or -1 if not found */
int VelocityI(int nHeight)
{
    if (FACTOR_TOGGLE(nHeight)) {
        VELOCITY_FACTOR = true;
    }
    return nHeight;
}

/* Velocity(int nHeight) ? true : false
   Returns true if nHeight is higher or equal to VELOCITY_HEIGHT */
bool Velocity_check(int nHeight)
{
    LogPrintf("Checking for Velocity on block %u: ",nHeight);
    if(VelocityI(nHeight) >= VELOCITY_HEIGHT[nHeight])
    {
        LogPrintf("Velocity is currently Enabled\n");
        return true;
    }
    LogPrintf("Velocity is currently disabled\n");
    return false;
}

/* Velocity(CBlockIndex* prevBlock, CBlock* block) ? true : false
   Goes close to the top of CBlock::AcceptBlock
   Returns true if proposed Block matches constrains */
bool Velocity(CBlockIndex* prevBlock, CBlock* block, bool fFactor_tx)
{
    // Define values
    int64_t TXrate = 0;
    int64_t CURvalstamp  = 0;
    int64_t OLDvalstamp  = 0;
    int64_t CURstamp = 0;
    int64_t OLDstamp = 0;
    int64_t TXstampC = 0;
    int64_t TXstampO = 0;
    int64_t SYScrntstamp = 0;
    int64_t SYSbaseStamp = 0;
    int nHeight = prevBlock->nHeight+1;
    int i = VelocityI(nHeight);
    // Set stanard values
    TXrate = block->GetBlockTime() - prevBlock->GetBlockTime();
    TXstampC = block->nTime;
    TXstampO = prevBlock->nTime;
    CURstamp = block->GetBlockTime();
    OLDstamp = prevBlock->GetBlockTime();
    CURvalstamp = prevBlock->GetBlockTime() + VELOCITY_MIN_RATE[i];
    OLDvalstamp = prevBlock->pprev->GetBlockTime() + VELOCITY_MIN_RATE[i];
    SYScrntstamp = GetAdjustedTime() + VELOCITY_MIN_RATE[i];
    SYSbaseStamp = GetTime() + VELOCITY_MIN_RATE[i];

    // Skip checks for Velocity activation block(s) (may not conform)
    if(nHeight == VELOCITY_HEIGHT[i])
    {
        LogPrintf("SKIPPED: Velocity activation toggle block(s)\n");
        return true;
    }

    // TODO: Clean this up!
    // Skip factoring for old switch-over blocks (may not conform)
    if((nHeight < 1024000 && nHeight > 981144)
            || nHeight == 1051651 || nHeight == 1052063
            || nHeight == 1061153 || nHeight == 1061319
            || nHeight == 1062201 || nHeight == 1065588
            || nHeight == 1069792 || nHeight == 1088564
            || nHeight == 1094595) {
        LogPrintf("NOTICE: Velocity disabled factoring for unsupported block(s)\n");
        fFactor_tx = false;
    }

    // Factor in TXs for Velocity constraints
    if(VELOCITY_FACTOR == true && fFactor_tx)
    {
        // Run TX factoring
        if(!tx_Factor(prevBlock, block))
        {
            LogPrintf("DENIED: Velocity denied block: %u\n", nHeight);
            return false;
        }
    }

    // Verify minimum Velocity rate
    if( VELOCITY_RATE[i] > 0 && TXrate >= VELOCITY_MIN_RATE[i] )
    {
        LogPrintf("CHECK_PASSED: block spacing has met Velocity constraints\n");
    }
    // Rates that are too rapid are rejected without exception
    else if( VELOCITY_RATE[i] > 0 && TXrate < VELOCITY_MIN_RATE[i] )
    {
        LogPrintf("DENIED: Minimum block spacing not met for Velocity\n");
        return false;
    }

    // Validate timestamp is logical based on previous block history
    if(CURstamp < CURvalstamp || TXstampC < CURvalstamp)
    {
        LogPrintf("DENIED: Block timestamp is not logical\n");
        return false;
    }
    else if(OLDstamp < OLDvalstamp || TXstampO < OLDvalstamp)
    {
        LogPrintf("DENIED: Block timestamp is not logical\n");
        return false;
    }

    // Validate timestamp is logical based on system time
    if(CURstamp > SYSbaseStamp || CURstamp > SYScrntstamp)
    {
        LogPrintf("DENIED: Block timestamp is not logical\n");
        return false;
    }
    else if(TXstampC > SYSbaseStamp || TXstampC > SYScrntstamp)
    {
        LogPrintf("DENIED: Block timestamp is not logical\n");
        return false;
    }

    // Constrain Velocity
    if(VELOCITY_EXPLICIT[i])
    {
        if(VELOCITY_MIN_TX[i] > 0)
            return false;
        if(VELOCITY_MIN_VALUE[i] > 0)
            return false;
        if(VELOCITY_MIN_FEE[i] > 0)
            return false;
    }
    // Velocity constraints met, return block acceptance
    return true;
}

bool RollingCheckpoints(int nHeight, CBlockIndex* pindexRequest)
{
    // Skip chain start
    if (nHeight < 5000) {
        return false;
    }
    // Define values
    CBlockIndex* pindexCurrentBlock = pindexRequest;
    CBlockIndex* pindexPastBlock = pindexCurrentBlock;
    // Set count and loop
    int pastBLOCK_1 = (nHeight - (BLOCK_TEMP_CHECKPOINT_DEPTH + BLOCK_REORG_THRESHOLD));
    while (pastBLOCK_1 < pindexCurrentBlock->nHeight) {
        // Index backwards
        pindexPastBlock = pindexPastBlock->pprev;
        pastBLOCK_1 ++;
    }
    // Set output values
    RollingBlock = pindexPastBlock->GetBlockHash();
    RollingHeight = pindexPastBlock->nHeight;
    // Success
    return true;
}

// Factor in TXs for Velocity constraints
bool tx_Factor(CBlockIndex* prevBlock, CBlock* block)
{
    // Define Values
    int64_t tx_outputs_values = 0;
    int64_t tx_MapOut_values = 0;
    int64_t tx_threshold = 0;
    CTxDB txdb("r");
    map<uint256, CTxIndex> mapQueuedChanges;

    // Set factor values
    BOOST_FOREACH(const CTransaction& tx, block->vtx)
    {
        // Load TX inputs
        MapPrevTx mapInputs;
        uint256 hashTx = tx.GetHash();
        bool fInvalid = false;
        // Don't run input checks for coinbase TX
        if (tx.IsCoinBase()) {
            tx_outputs_values += tx.GetValueOut();
        } else {
            // Ensure we can fetch inputs
            if (!tx.FetchInputs(txdb, mapQueuedChanges, false, true, mapInputs, fInvalid)) {
                LogPrintf("DENIED: Invalid TX found during FetchInputs\n");
                return false;
            }
            // Authenticate submitted block's TXs
            tx_MapOut_values = tx.GetValueOut();
            if(tx_outputs_values + tx_MapOut_values >= 0) {
                tx_outputs_values += tx_MapOut_values;
            } else {
                LogPrintf("DENIED: overflow detected tx_outputs_values + tx.GetValueOut()\n");
                return false;
            }
        }
        mapQueuedChanges[hashTx] = CTxIndex(CDiskTxPos(1,1,1), tx.vout.size());
    }

    // Set threshold values
    if(block->IsProofOfStake()) {
        tx_threshold = GetProofOfStakeReward(0, 0);
    } else {
        tx_threshold = GetProofOfWorkReward(prevBlock->nHeight+1, 0);
    }

    // Ensure input/output sanity of transactions in the block
    if((prevBlock->nMoneySupply + tx_threshold) < tx_outputs_values)
    {
        LogPrintf("DENIED: block contains a tx input that is less that output\n");
        return false;
    }

    // Return success if we get here
    LogPrintf("CHECK_PASSED: transaction/input factoring has met Velocity constraints\n");
    return true;
}

bool bIndex_Factor(CBlockIndex* InSplitPoint, CBlockIndex* InSplitEnd)
{
    // Ensure expected coin supply matches actualy coin supply of branch
    if((InSplitPoint->nMoneySupply / COIN) < (InSplitEnd->nMoneySupply / COIN))
    {
        LogPrintf("VELOCITY_FACTOR: Mismatched supply in branch, excpected: %u | found: %u\n", (int64_t)(InSplitPoint->nMoneySupply / COIN), (int64_t)(InSplitEnd->nMoneySupply / COIN));
        LogPrintf("DENIED: branch contains invalid coin supply amount\n");
        return false;
    }
    return true;
}
