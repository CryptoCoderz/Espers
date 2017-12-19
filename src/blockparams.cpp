// Copyright (c) 2016-2017 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "blockparams.h"
#include "chainparams.h"
#include "checkpoints.h"
#include "db.h"
#include "fork.h"
#include "init.h"
#include "kernel.h"
#include "net.h"
#include "txdb.h"
#include "velocity.h"
#include "main.h"

using namespace std;
using namespace boost;

//////////////////////////////////////////////////////////////////////////////
//
// Standard Global Values
//

//
// Section defines global values for retarget logic
//

double VLF1 = 0;
double VLF2 = 0;
double VLF3 = 0;
double VLF4 = 0;
double VLF5 = 0;
double VLFtmp = 0;
double VRFsm1 = 1;
double VRFdw1 = 0.75;
double VRFdw2 = 0.5;
double VRFup1 = 1.25;
double VRFup2 = 1.5;
double VRFup3 = 2;
double TerminalAverage = 0;
double TerminalFactor = 10000;
CBigNum newBN = 0;
CBigNum oldBN = 0;
int64_t VLrate1 = 0;
int64_t VLrate2 = 0;
int64_t VLrate3 = 0;
int64_t VLrate4 = 0;
int64_t VLrate5 = 0;
int64_t VLRtemp = 0;
int64_t DSrateNRM = BLOCK_SPACING;
int64_t DSrateMAX = BLOCK_SPACING_MAX;
int64_t FRrateDWN = DSrateNRM - 60;
int64_t FRrateFLR = DSrateNRM - 80;
int64_t FRrateCLNG = DSrateMAX * 3;
int64_t difficultyfactor = 0;
int64_t AverageDivisor = 5;
int64_t scanheight = 6;
int64_t scanblocks = 1;
int64_t scantime_1 = 0;
int64_t scantime_2 = 0;
int64_t prevPoW = 0; // hybrid value
int64_t prevPoS = 0; // hybrid value
const CBlockIndex* pindexPrev = 0;
const CBlockIndex* BlockVelocityType = 0;
CBigNum bnOld;
CBigNum bnNew;
unsigned int retarget = DIFF_VRX; // Default with VRX
uint64_t cntTime = 0;
uint64_t prvTime = 0;
uint64_t difTimePoS = 0;
uint64_t difTimePoW = 0;


//////////////////////////////////////////////////////////////////////////////
//
// Debug section
//

//
// Debug log printing
//

void VRXswngPoSdebug()
{
    // Print for debugging
    LogPrintf("Previously discovered PoS block: %u: \n",prvTime);
    LogPrintf("Current block-time: %u: \n",cntTime);
    LogPrintf("Time since last PoS block: %u: \n",difTimePoS);
    if(difTimePoS > 1 * 60 * 60) { TerminalAverage /= 2; LogPrintf("diffTimePoS is greater than 1 Hours: %u \n",cntTime);}
    if(difTimePoS > 2 * 60 * 60) { TerminalAverage /= 2; LogPrintf("diffTimePoS is greater than 2 Hours: %u \n",cntTime);}
    if(difTimePoS > 3 * 60 * 60) { TerminalAverage /= 2; LogPrintf("diffTimePoS is greater than 3 Hours: %u \n",cntTime);}
    if(difTimePoS > 4 * 60 * 60) { TerminalAverage /= 2; LogPrintf("diffTimePoS is greater than 4 Hours: %u \n",cntTime);}
    return;
}

void VRXswngPoWdebug()
{
    // Print for debugging
    LogPrintf("Previously discovered PoW block: %u: \n",prvTime);
    LogPrintf("Current block-time: %u: \n",cntTime);
    LogPrintf("Time since last PoW block: %u: \n",difTimePoW);
    if(difTimePoW > 1 * 60 * 60) { TerminalAverage /= 2; LogPrintf("diffTimePoW is greater than 1 Hours: %u \n",cntTime);}
    if(difTimePoW > 2 * 60 * 60) { TerminalAverage /= 2; LogPrintf("diffTimePoW is greater than 2 Hours: %u \n",cntTime);}
    if(difTimePoW > 3 * 60 * 60) { TerminalAverage /= 2; LogPrintf("diffTimePoW is greater than 3 Hours: %u \n",cntTime);}
    if(difTimePoW > 4 * 60 * 60) { TerminalAverage /= 2; LogPrintf("diffTimePoW is greater than 4 Hours: %u \n",cntTime);}
    return;
}

void VRXdebug()
{
    // Print for debugging
    LogPrintf("Terminal-Velocity 1st spacing: %u: \n",VLrate1);
    LogPrintf("Terminal-Velocity 2nd spacing: %u: \n",VLrate2);
    LogPrintf("Terminal-Velocity 3rd spacing: %u: \n",VLrate3);
    LogPrintf("Terminal-Velocity 4th spacing: %u: \n",VLrate4);
    LogPrintf("Terminal-Velocity 5th spacing: %u: \n",VLrate5);
    LogPrintf("Desired normal spacing: %u: \n",DSrateNRM);
    LogPrintf("Desired maximum spacing: %u: \n",DSrateMAX);
    LogPrintf("Terminal-Velocity 1st multiplier set to: %f: \n",VLF1);
    LogPrintf("Terminal-Velocity 2nd multiplier set to: %f: \n",VLF2);
    LogPrintf("Terminal-Velocity 3rd multiplier set to: %f: \n",VLF3);
    LogPrintf("Terminal-Velocity 4th multiplier set to: %f: \n",VLF4);
    LogPrintf("Terminal-Velocity 5th multiplier set to: %f: \n",VLF5);
    LogPrintf("Terminal-Velocity averaged a final multiplier of: %f: \n",TerminalAverage);
    LogPrintf("Prior Terminal-Velocity: %u\n", oldBN);
    LogPrintf("New Terminal-Velocity:  %u\n", newBN);
    return;
}

void GNTdebug()
{
    // Print for debugging
    // Retarget using DGW-v3
    if (retarget == DIFF_DGW)
    {
        // debug info for testing
        LogPrintf("DarkGravityWave-v3 retarget selected \n");
        LogPrintf("Espers retargetted using: DGW-v3 difficulty algo \n");
        return;
    }
    // Retarget using PPC
    else if (retarget == DIFF_PPC)
    {
        // debug info for testing
        LogPrintf("PPC per-block retarget selected \n");
        LogPrintf("Espers retargetted using: PPC difficulty algo \n");
        return;
    }
    // Retarget using Terminal-Velocity
    // debug info for testing
    LogPrintf("Terminal-Velocity retarget selected \n");
    LogPrintf("Espers retargetted using: Terminal-Velocity difficulty curve \n");
    return;
}


//////////////////////////////////////////////////////////////////////////////
//
// Difficulty retarget (depricated section)
//

//
// These are the depricated and no longer in use retarget solutions
//

unsigned int PeercoinDiff(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    // Standard PPC retarget system, similiar to BTC's
    CBigNum bnTargetLimit = fProofOfStake ? Params().ProofOfStakeLimit() : Params().ProofOfWorkLimit();

    if (pindexLast == NULL)
        return bnTargetLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // second block

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
    if (nActualSpacing < 0)
        nActualSpacing = Params().TargetSpacing();

    // ppcoin: target change every block
    // ppcoin: retarget with exponential moving toward target spacing
    bnNew.SetCompact(pindexPrev->nBits);
    int64_t nInterval = Params().TargetTimespan() / Params().TargetSpacing();
    bnNew *= ((nInterval - 1) * Params().TargetSpacing() + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * Params().TargetSpacing());

    if (bnNew <= 0 || bnNew > bnTargetLimit)
        bnNew = bnTargetLimit;

    return bnNew.GetCompact();
}

unsigned int DarkGravityWave(const CBlockIndex* pindexLast, bool fProofOfStake)
{
        // DarkGravityWave v3.1, written by Evan Duffield - evan@dashpay.io
        // Modified & revised by bitbandi for PoW support [implementation (fork) cleanup done by CryptoCoderz]
        const CBigNum nProofOfWorkLimit = fProofOfStake ? Params().ProofOfStakeLimit() : Params().ProofOfWorkLimit();
        const CBlockIndex *BlockLastSolved = pindexLast;
        const CBlockIndex *BlockLastSolved_lgf = GetLastBlockIndex(pindexLast, fProofOfStake);
        const CBlockIndex *BlockReading = pindexLast;
        // Low Gravity fix (PoW support)
        if(pindexBest->nHeight > nlowGravity)
        {
            BlockReading = BlockLastSolved_lgf;
        }
        int64_t nActualTimespan = 0;
        int64_t LastBlockTime = 0;
        int64_t PastBlocksMin = 7;
        int64_t PastBlocksMax = 24;
        int64_t CountBlocks = 0;
        CBigNum PastDifficultyAverage;
        CBigNum PastDifficultyAveragePrev;

        if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || BlockLastSolved->nHeight < PastBlocksMax) {
            return nProofOfWorkLimit.GetCompact();
        }

        for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
            if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
            CountBlocks++;

            if(CountBlocks <= PastBlocksMin) {
                if (CountBlocks == 1) { PastDifficultyAverage.SetCompact(BlockReading->nBits); }
                else { PastDifficultyAverage = ((PastDifficultyAveragePrev * CountBlocks) + (CBigNum().SetCompact(BlockReading->nBits))) / (CountBlocks + 1); }
                PastDifficultyAveragePrev = PastDifficultyAverage;
            }

            if(LastBlockTime > 0){
                int64_t Diff = (LastBlockTime - BlockReading->GetBlockTime());
                nActualTimespan += Diff;
            }
            LastBlockTime = BlockReading->GetBlockTime();
            // Low Gravity chain support (Pre-fix)
            if(pindexBest->nHeight <= nlowGravity)
            {
                if (BlockReading->pprev == NULL) { assert(BlockReading); break; }
                    BlockReading = BlockReading->pprev;
            }
            // Low Gravity fix (PoW support)
            else if(pindexBest->nHeight > nlowGravity)
            {
                BlockReading = GetLastBlockIndex(BlockReading->pprev, fProofOfStake);
            }
        }

        CBigNum bnNew(PastDifficultyAverage);

        int64_t _nTargetTimespan = CountBlocks * Params().TargetSpacing();

        if (nActualTimespan < _nTargetTimespan/3)
            nActualTimespan = _nTargetTimespan/3;
        if (nActualTimespan > _nTargetTimespan*3)
            nActualTimespan = _nTargetTimespan*3;

        // Retarget
        bnNew *= nActualTimespan;
        bnNew /= _nTargetTimespan;

        if (bnNew > nProofOfWorkLimit){
            bnNew = nProofOfWorkLimit;
        }

        return bnNew.GetCompact();
}

//////////////////////////////////////////////////////////////////////////////
//
// Difficulty retarget (current section)
//

//
// This is VRX revised implementation
//
// Terminal-Velocity-RateX, v10-Beta-R7, written by Jonathan Dan Zaretsky - cryptocoderz@gmail.com
void VRX_BaseEngine(const CBlockIndex* pindexLast, bool fProofOfStake)
{
       // Set base values
       VLF1 = 0;
       VLF2 = 0;
       VLF3 = 0;
       VLF4 = 0;
       VLF5 = 0;
       VLFtmp = 0;
       TerminalAverage = 0;
       TerminalFactor = 10000;
       VLrate1 = 0;
       VLrate2 = 0;
       VLrate3 = 0;
       VLrate4 = 0;
       VLrate5 = 0;
       VLRtemp = 0;
       difficultyfactor = 0;
       scanblocks = 1;
       scantime_1 = 0;
       scantime_2 = pindexLast->GetBlockTime();
       prevPoW = 0; // hybrid value
       prevPoS = 0; // hybrid value
       // Set prev blocks...
       pindexPrev = pindexLast;
       // ...and deduce spacing
       while(scanblocks < scanheight)
       {
           scantime_1 = scantime_2;
           pindexPrev = pindexPrev->pprev;
           scantime_2 = pindexPrev->GetBlockTime();
           // Set standard values
           if(scanblocks > 0){
               if     (scanblocks < scanheight-4){ VLrate1 = (scantime_1 - scantime_2); VLRtemp = VLrate1; }
               else if(scanblocks < scanheight-3){ VLrate2 = (scantime_1 - scantime_2); VLRtemp = VLrate2; }
               else if(scanblocks < scanheight-2){ VLrate3 = (scantime_1 - scantime_2); VLRtemp = VLrate3; }
               else if(scanblocks < scanheight-1){ VLrate4 = (scantime_1 - scantime_2); VLRtemp = VLrate4; }
               else if(scanblocks < scanheight-0){ VLrate5 = (scantime_1 - scantime_2); VLRtemp = VLrate5; }
           }
           // Round factoring
           if(VLRtemp >= DSrateNRM){ VLFtmp = VRFsm1;
               if(VLRtemp > DSrateMAX){ VLFtmp = VRFdw1;
                   if(VLRtemp > FRrateCLNG){ VLFtmp = VRFdw2; }
               }
           }
           else if(VLRtemp < DSrateNRM){ VLFtmp = VRFup1;
               if(VLRtemp < FRrateDWN){ VLFtmp = VRFup2;
                   if(VLRtemp < FRrateFLR){ VLFtmp = VRFup3; }
               }
           }
           // Record factoring
           if      (scanblocks < scanheight-4) VLF1 = VLFtmp;
           else if (scanblocks < scanheight-3) VLF2 = VLFtmp;
           else if (scanblocks < scanheight-2) VLF3 = VLFtmp;
           else if (scanblocks < scanheight-1) VLF4 = VLFtmp;
           else if (scanblocks < scanheight-0) VLF5 = VLFtmp;
           // Log hybrid block type
           //
           // v1.0
           if(pindexBest->GetBlockTime() < SWING_PATCH) // ON (TOGGLED 11/01/2017)
           {
                if     (fProofOfStake) prevPoS ++;
                else if(!fProofOfStake) prevPoW ++;
           }
           // v1.1
           if(pindexBest->GetBlockTime() > SWING_PATCH) // ON (TOGGLED 11/01/2017)
           {
               if(pindexPrev->IsProofOfStake())
               {
                   prevPoS ++;
               }
               else if(pindexPrev->IsProofOfWork())
               {
                   prevPoW ++;
               }
           }

           // move up per scan round
           scanblocks ++;
       }
       // Final mathematics
       TerminalAverage = (VLF1 + VLF2 + VLF3 + VLF4 + VLF5) / AverageDivisor;
       return;
}

void VRX_ThreadCurve(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    // Run VRX engine
    VRX_BaseEngine(pindexLast, fProofOfStake);

    //
    // Skew for less selected block type
    //

    // Version 1.0
    //
    int64_t nNow = nBestHeight; int64_t nThen = sysUpgrade_01; // Toggle skew system fork
    if(nNow > nThen){if(prevPoW < prevPoS && !fProofOfStake){if((prevPoS-prevPoW) > 3) TerminalAverage /= 3;}
    else if(prevPoW > prevPoS && fProofOfStake){if((prevPoW-prevPoS) > 3) TerminalAverage /= 3;}
    if(TerminalAverage < 0.5) TerminalAverage = 0.5;} // limit skew to halving

    // Version 1.1 curve-patch
    //
    if(pindexBest->GetBlockTime() > SWING_PATCH) // ON (TOGGLED 11/01/2017)
    {
        // Define time values
        cntTime = BlockVelocityType->GetBlockTime();
        prvTime = BlockVelocityType->pprev->GetBlockTime();

        if(fProofOfStake)
        {
            difTimePoS = cntTime - prvTime;

            // Debug print toggle
            if(fDebug) VRXswngPoSdebug();
            // Normal Run
            else if(!fDebug)
            {
                if(difTimePoS > 1 * 60 * 60) { TerminalAverage /= 2; }
                if(difTimePoS > 2 * 60 * 60) { TerminalAverage /= 2; }
                if(difTimePoS > 3 * 60 * 60) { TerminalAverage /= 2; }
                if(difTimePoS > 4 * 60 * 60) { TerminalAverage /= 2; }
            }
        }
        else if(!fProofOfStake)
        {
            difTimePoW = cntTime - prvTime;

            // Debug print toggle
            if(fDebug) VRXswngPoWdebug();
            // Normal Run
            else if(!fDebug)
            {
                if(difTimePoW > 1 * 60 * 60) { TerminalAverage /= 2; }
                if(difTimePoW > 2 * 60 * 60) { TerminalAverage /= 2; }
                if(difTimePoW > 3 * 60 * 60) { TerminalAverage /= 2; }
                if(difTimePoW > 4 * 60 * 60) { TerminalAverage /= 2; }
            }
        }
    }
    return;
}

unsigned int VRX_Retarget(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    const CBigNum bnVelocity = fProofOfStake ? Params().ProofOfStakeLimit() : Params().ProofOfWorkLimit();

    // Check for blocks to index
    if (pindexLast->nHeight < scanheight)
        return bnVelocity.GetCompact(); // can't index prevblock

    // Differentiate PoW/PoS prev block
    BlockVelocityType = GetLastBlockIndex(pindexLast, fProofOfStake);

    // Run VRX threadcurve
    VRX_ThreadCurve(pindexLast, fProofOfStake);

    // Check for stall
    // Depricated as of DEC/15/2017 until futher notice
    if(nBestHeight < 704195)
    {
        if(bnNew.GetCompact() == bnVelocity.GetCompact())
        {
            LogPrintf("Diff restarted DUE TO STALL \n");
            return bnVelocity.GetCompact(); // restart thread diff
        }
    }

    // Force fork block min diff
    if(nBestHeight == 704194)
        return bnVelocity.GetCompact(); // restart thread diff
    // Force fork block min diff (PoW)
    if(nBestHeight > 704308 && nBestHeight < 704311)
    {
        if(!fProofOfStake)
            return bnVelocity.GetCompact(); // restart thread diff
    }

    // Retarget
    TerminalFactor *= TerminalAverage;
    difficultyfactor = TerminalFactor;
    bnOld.SetCompact(BlockVelocityType->nBits);
    bnNew = bnOld / difficultyfactor;
    bnNew *= 10000;

    // Limit
    if (bnNew > bnVelocity)
      bnNew = bnVelocity;

    // Final log
    oldBN = bnOld.GetCompact();
    newBN = bnNew.GetCompact();

    // Debug print toggle
    if(fDebug) VRXdebug();

    // Return difficulty
    return bnNew.GetCompact();
}

//////////////////////////////////////////////////////////////////////////////
//
// Difficulty retarget (function)
//

unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    /* DarkGravityWave v3 retarget difficulty runs until block 667350 */
    if(pindexBest->nHeight < VELOCITY_TDIFF)
    {
        retarget = DIFF_DGW;
    }
    /* Chain starts with Peercoin per-block restarget,
       PPC retarget difficulty runs for the initial 615k (thousand) blocks */
    if(pindexBest->nHeight < nGravityFork)
    {
        retarget = DIFF_PPC;
    }
    // Retarget using DGW-v3
    if (retarget == DIFF_DGW)
    {
        // debug info for testing
        if(fDebug) GNTdebug();
        return DarkGravityWave(pindexLast, fProofOfStake);
    }
    // Retarget using PPC
    else if (retarget == DIFF_PPC)
    {
        // debug info for testing
        if(fDebug) GNTdebug();
        return PeercoinDiff(pindexLast, fProofOfStake);
    }
    // Retarget using Terminal-Velocity
    // debug info for testing
    if(fDebug) GNTdebug();
    return VRX_Retarget(pindexLast, fProofOfStake);
}

//////////////////////////////////////////////////////////////////////////////
//
// Coin base subsidy
//

//
// PoW coin base reward
//
int64_t GetProofOfWorkReward(int64_t nHeight, int64_t nFees)
{
    int64_t nSubsidy = nBlockPoWReward;
    // Genesis block subsidy
    if(nHeight == nGenesisHeight) {
        return  nGenesisBlockReward;
    }
    // Block Buffer
    else if (nHeight <= nReservePhaseStart) {
        nSubsidy = nBlockRewardBuffer;
    }
    // Reserved for Swap from Initial Espers Chain Launch
    else if(nHeight > nReservePhaseStart) {
        if(nHeight < nReservePhaseEnd){
        nSubsidy = nBlockRewardReserve;
        }
    }
    // hardCap v2.1
    else if(pindexBest->nMoneySupply > MAX_SINGLE_TX)
    {
        LogPrint("MINEOUT", "GetProofOfWorkReward(): create=%s nFees=%d\n", FormatMoney(nFees), nFees);
        return nFees;
    }

    LogPrint("creation", "GetProofOfWorkReward() : create=%s nSubsidy=%d\n", FormatMoney(nSubsidy), nSubsidy);
    return nSubsidy + nFees;
}

//
// PoS coin base reward
//
int64_t GetProofOfStakeReward(int64_t nCoinAge, int64_t nFees)
{
    int64_t nSubsidy = nCoinAge * COIN_YEAR_REWARD * 33 / (365 * 33 + 8);

    if(pindexBest->nHeight > nPoS1PhaseStart){
    nSubsidy = nCoinAge * COIN_YEAR_REWARD4 * 33 / (365 * 33 + 8);
    }
    // Previously nBestHeight
    else if(pindexBest->nHeight > nPoS5PhaseStart){
    nSubsidy = nCoinAge * COIN_YEAR_REWARD3 * 33 / (365 * 33 + 8);
    }
    // Previously nBestHeight
    else if(pindexBest->nHeight > nPoS25PhaseStart){
    nSubsidy = nCoinAge * COIN_YEAR_REWARD2 * 33 / (365 * 33 + 8);
    }
    // hardCap v2.1
    else if(pindexBest->nMoneySupply > MAX_SINGLE_TX)
    {
        LogPrint("MINEOUT", "GetProofOfStakeReward(): create=%s nFees=%d\n", FormatMoney(nFees), nFees);
        return nFees;
    }

    LogPrint("creation", "GetProofOfStakeReward(): create=%s nCoinAge=%d\n", FormatMoney(nSubsidy), nCoinAge);
    return nSubsidy + nFees;
}
