// Copyright (c) 2014 The Cryptocoin Revival Foundation
// Copyright (c) 2015-2019 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "main.h"
#include "velocity.h"
#include "rpcserver.h"

/* VelocityI(int nHeight) ? i : -1
   Returns i or -1 if not found */
int VelocityI(int nHeight)
{
    int i = 0;
    i --;
    BOOST_FOREACH(int h, VELOCITY_HEIGHT)
    if( nHeight >= h )
      i++;
    return i;
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
    const MapPrevTx mapInputs;

    // Define values
    int64_t TXvalue = 0;
    int64_t TXinput = 0;
    int64_t TXfee = 0;
    int64_t TXcount = 0;
    int64_t TXlogic = 0;
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
    int HaveCoins = false;
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

    // TODO: Rework and activate below section for future releases
    // Factor in TXs for Velocity constraints only if there are TXs to do so with
    if(VELOCITY_FACTOR[i] == true && TXvalue > 0)
    {
        // Set factor values
        BOOST_FOREACH(const CTransaction& tx, block->vtx)
        {
            TXvalue = tx.GetValueOut();
            TXinput = tx.GetValueIn(mapInputs);
            TXfee = TXinput - TXvalue;
            TXcount = block->vtx.size();
            TXlogic = GetPrevAccountBalance - TXinput;
            // TXrate = block->GetBlockTime() - prevBlock->GetBlockTime();
        }
        // Set Velocity logic value
        if(TXlogic > 0)
        {
            HaveCoins = true;
        }
        // Check for and enforce minimum TXs per block (Minimum TXs are disabled for Espers)
        if(VELOCITY_MIN_TX[i] > 0 && TXcount < VELOCITY_MIN_TX[i])
        {
            LogPrintf("DENIED: Not enough TXs in block\n");
            return false;
        }
        // Authenticate submitted block's TXs
        if(VELOCITY_MIN_VALUE[i] > 0 || VELOCITY_MIN_FEE[i] > 0)
        {
            // Make sure we accept only blocks that sent an amount
            // NOT being more than available coins to send
            if(VELOCITY_MIN_FEE[i] > 0 && TXinput > 0)
            {
                if(HaveCoins == false)
                {
                    LogPrintf("DENIED: Balance has insuficient funds for attempted TX with Velocity\n");
                    return false;
                }
            }

            if(VELOCITY_MIN_VALUE[i] > 0 && TXvalue < VELOCITY_MIN_VALUE[i])
            {
                LogPrintf("DENIED: Invalid TX value found by Velocity\n");
                return false;
            }
            if(VELOCITY_MIN_FEE[i] > 0 && TXinput > 0)
            {
                if(TXfee < VELOCITY_MIN_FEE[i])
                {
                    LogPrintf("DENIED: Invalid network fee found by Velocity\n");
                    return false;
                }
            }
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
