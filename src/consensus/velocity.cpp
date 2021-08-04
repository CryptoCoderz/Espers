// Copyright (c) 2014 The Cryptocoin Revival Foundation
// Copyright (c) 2015-2021 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core/main.h"
#include "database/txdb.h"
#include "velocity.h"
#include "rpc/rpcserver.h"
#include "core/blockparams.h"

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
    if(VelocityI(nHeight) >= 0)
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
bool Velocity(CBlockIndex* prevBlock, CBlock* block)
{
    // Define values
    CAmount tx_inputs_values = 0;
    CAmount tx_outputs_values = 0;
    CAmount tx_MapIn_values = 0;
    CAmount tx_MapOut_values = 0;
    CAmount tx_threshold = 0;
    int64_t TXcount = block->vtx.size();
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

    if(block->IsProofOfStake()) {
        tx_threshold = GetProofOfStakeReward(0, 0);
    } else {
        tx_threshold = GetProofOfWorkReward(pindexBest->nHeight, 0);
    }

    // Factor in TXs for Velocity constraints
    if(VELOCITY_FACTOR == true)
    {
        // Set factor values
        BOOST_FOREACH(const CTransaction& tx, block->vtx)
        {
            // Load TX inputs
            CTxDB txdb("r");
            MapPrevTx mapInputs;
            map<uint256, CTxIndex> mapUnused;
            bool fInvalid = false;
            bool fNoIn = false;
            // Ensure we can fetch inputs
            if (!tx.FetchInputs(txdb, mapUnused, true, false, mapInputs, fInvalid))
            {
                if(nHeight > 982300) {
                    LogPrintf("DENIED: Invalid TX found during FetchInputs\n");
                    return false;
                } else {
                    LogPrintf("WARNING: Potentially Invalid TX found during FetchInputs\n");
                    fNoIn = true;
                }
            }
            // Authenticate submitted block's TXs
            if(!fNoIn) {
                tx_MapIn_values = tx.GetValueMapIn(mapInputs);
                if(tx_inputs_values + tx_MapIn_values >= 0) {
                    tx_inputs_values += tx_MapIn_values;
                } else {
                    LogPrintf("DENIED: overflow detected tx_inputs_values + tx.GetValueMapIn(mapInputs)\n");
                    return false;
                }
            }
            tx_MapOut_values = tx.GetValueOut();
            if(tx_outputs_values + tx_MapOut_values >= 0) {
                tx_outputs_values += tx_MapOut_values;
            } else {
                LogPrintf("DENIED: overflow detected tx_outputs_values + tx.GetValueOut()\n");
                return false;
            }
        }
        // Ensure input/output sanity of transactions in the block
        if((tx_inputs_values + tx_threshold) < tx_outputs_values)
        {
            if(nHeight < 981135 || nHeight > 982300) {// skip trouble blocks during initial rushed patch
                LogPrintf("DENIED: block contains a tx input that is less that output\n");
                return false;
            }
        }
        // Ensure expected coin supply matches actualy coin supply of block
        if((prevBlock->nMoneySupply + tx_threshold) < (tx_outputs_values))
        {
            LogPrintf("DENIED: block contains invalid coin supply amount\n");
            return false;
        }
        // Check for and enforce minimum TXs per block (Minimum TXs are disabled for Espers)
        if(VELOCITY_MIN_TX[i] > 0 && TXcount < VELOCITY_MIN_TX[i])
        {
            LogPrintf("DENIED: Not enough TXs in block\n");
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
        if(nHeight != VELOCITY_HEIGHT[i])
        {
            LogPrintf("DENIED: Block timestamp is not logical\n");
            return false;
        }
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

bool RollingCheckpoints(int nHeight)
{
    // Skip chain start
    if (nHeight < 5000) {
        return false;
    }
    // Define values
    CBlockIndex* pindexCurrentBlock = pindexBest;
    CBlockIndex* pindexPastBlock = pindexCurrentBlock;
    // Set count and loop
    int pastBLOCK_1 = (pindexCurrentBlock->nHeight - BLOCK_TEMP_CHECKPOINT_DEPTH);
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
