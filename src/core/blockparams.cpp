// Copyright (c) 2016-2021 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "blockparams.h"
#include "chainparams.h"
#include "consensus/checkpoints.h"
#include "database/db.h"
#include "consensus/fork.h"
#include "util/init.h"
#include "consensus/kernel.h"
#include "node/net.h"
#include "database/txdb.h"
#include "consensus/velocity.h"
#include "primitives/uint256.h"
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
double debugTerminalAverage = 0;
uint256 newBN = 0;
uint256 oldBN = 0;
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
uint64_t blkTime = 0;
uint64_t cntTime = 0;
uint64_t prvTime = 0;
uint64_t difTime = 0;
uint64_t hourRounds = 0;
uint64_t difCurve = 0;
uint64_t debugHourRounds = 0;
uint64_t debugDifCurve = 0;
bool fDryRun;
bool fCRVreset;
const CBlockIndex* pindexPrev = 0;
const CBlockIndex* BlockVelocityType = 0;
uint256 bnVelocity = 0;
uint256 bnOld;
uint256 bnNew;
std::string difType ("");
unsigned int retarget = DIFF_VRX; // Default with VRX


//////////////////////////////////////////////////////////////////////////////
//
// Debug section
//

//
// Debug log printing
//

void VRXswngdebug()
{
    // Print for debugging
    LogPrintf("Previously discovered %s block: %u: \n",difType.c_str(),prvTime);
    LogPrintf("Current block-time: %u: \n",difType.c_str(),cntTime);
    LogPrintf("Time since last %s block: %u: \n",difType.c_str(),difTime);
    // Handle updated versions as well as legacy
    if(GetTime() > 9993058800) {
        debugHourRounds = hourRounds;
        debugTerminalAverage = TerminalAverage;
        debugDifCurve = difCurve;
        while(difTime > (debugHourRounds * 60 * 60)) {
            debugTerminalAverage /= debugDifCurve;
            LogPrintf("diffTime%s is greater than %u Hours: %u \n",difType.c_str(),debugHourRounds,cntTime);
            LogPrintf("Difficulty will be multiplied by: %d \n",debugTerminalAverage);
            // Break loop after 5 hours, otherwise time threshold will auto-break loop
            if (debugHourRounds > 5){
                break;
            }
            debugDifCurve ++;
            debugHourRounds ++;
        }
    } else {
        if(difTime > (hourRounds+0) * 60 * 60) {LogPrintf("diffTime%s is greater than 1 Hours: %u \n",difType.c_str(),cntTime);}
        if(difTime > (hourRounds+1) * 60 * 60) {LogPrintf("diffTime%s is greater than 2 Hours: %u \n",difType.c_str(),cntTime);}
        if(difTime > (hourRounds+2) * 60 * 60) {LogPrintf("diffTime%s is greater than 3 Hours: %u \n",difType.c_str(),cntTime);}
        if(difTime > (hourRounds+3) * 60 * 60) {LogPrintf("diffTime%s is greater than 4 Hours: %u \n",difType.c_str(),cntTime);}
    }
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
    LogPrintf("Prior Terminal-Velocity: %u: \n", oldBN.GetLow64());
    LogPrintf("New Terminal-Velocity:  %u: \n", newBN.GetLow64());
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
    uint256 bnTargetLimit = fProofOfStake ? Params().ProofOfStakeLimit() : Params().ProofOfWorkLimit();

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
        const uint256 nProofOfWorkLimit = fProofOfStake ? Params().ProofOfStakeLimit() : Params().ProofOfWorkLimit();
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
        uint256 PastDifficultyAverage;
        uint256 PastDifficultyAveragePrev;

        if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || BlockLastSolved->nHeight < PastBlocksMax) {
            return nProofOfWorkLimit.GetCompact();
        }

        for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
            if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
            CountBlocks++;

            if(CountBlocks <= PastBlocksMin) {
                if (CountBlocks == 1) { PastDifficultyAverage.SetCompact(BlockReading->nBits); }
                else { PastDifficultyAverage = ((PastDifficultyAveragePrev * CountBlocks) + (uint256().SetCompact(BlockReading->nBits))) / (CountBlocks + 1); }
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

        uint256 bnNew(PastDifficultyAverage);

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
// This is VRX (v3.5) revised implementation
//
// Terminal-Velocity-RateX, v10-Beta-R9, written by Jonathan Dan Zaretsky - cryptocoderz@gmail.com
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
           if(pindexBest->GetBlockTime() < SWING_PATCH) // ON (TOGGLED Nov/01/2017)
           {
                if     (fProofOfStake) prevPoS ++;
                else if(!fProofOfStake) prevPoW ++;
           }
           // v1.1
           if(pindexBest->GetBlockTime() > SWING_PATCH) // ON (TOGGLED Nov/01/2017)
           {
               if(pindexPrev->IsProofOfStake()) { prevPoS ++; }
               else if(pindexPrev->IsProofOfWork()) { prevPoW ++; }
           }

           // move up per scan round
           scanblocks ++;
       }
       // Final mathematics
       TerminalAverage = (VLF1 + VLF2 + VLF3 + VLF4 + VLF5) / AverageDivisor;
       return;
}

void VRX_Simulate_Retarget()
{
    // Perform retarget simulation
    TerminalFactor *= TerminalAverage;
    difficultyfactor = TerminalFactor;
    bnOld.SetCompact(BlockVelocityType->nBits);
    bnNew = bnOld / difficultyfactor;
    bnNew *= 10000;
    // Reset TerminalFactor for actual retarget
    TerminalFactor = 10000;
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
    if(pindexBest->GetBlockTime() > SWING_PATCH) // ON (TOGGLED Nov/01/2017)
    {
        // Define time values
        blkTime = pindexLast->GetBlockTime();
        cntTime = BlockVelocityType->GetBlockTime();
        prvTime = BlockVelocityType->pprev->GetBlockTime();
        difTime = cntTime - prvTime;
        hourRounds = 1;
        difCurve = 2;
        fCRVreset = false;

        // Debug print toggle
        if(fProofOfStake) {
            difType = "PoS";
        } else {
            difType = "PoW";
        }
        if(fDebug) VRXswngdebug();

        // Version 1.2 Extended Curve Run Upgrade
        if(pindexLast->nHeight+1 >= nLiveForkToggle && nLiveForkToggle != 0) {
            // Set unbiased comparison
            difTime = blkTime - cntTime;
            // Run Curve
            while(difTime > (hourRounds * 60 * 60)) {
                // Break loop after 5 hours, otherwise time threshold will auto-break loop
                if (hourRounds > 5){
                    fCRVreset = true;
                    break;
                }
                // Drop difficulty per round
                TerminalAverage /= difCurve;
                // Simulate retarget for sanity
                VRX_Simulate_Retarget();
                // Increase Curve per round
                difCurve ++;
                // Move up an hour per round
                hourRounds ++;
            }
        } else {// Version 1.1 Standard Curve Run
            if(difTime > (hourRounds+0) * 60 * 60) { TerminalAverage /= difCurve; }
            if(difTime > (hourRounds+1) * 60 * 60) { TerminalAverage /= difCurve; }
            if(difTime > (hourRounds+2) * 60 * 60) { TerminalAverage /= difCurve; }
            if(difTime > (hourRounds+3) * 60 * 60) { TerminalAverage /= difCurve; }
        }
    }
    return;
}

void VRX_Dry_Run(const CBlockIndex* pindexLast)
{
    // Check for blocks to index
    if (pindexLast->nHeight < scanheight) {
        fDryRun = true;
        return; // can't index prevblock
    }

    // Check for stall
    // Depricated as of DEC/15/2017 until futher notice
    if(nBestHeight < 729000) // ON -- TODO - ADJUST DATE!
    {
        if(nBestHeight >= 704195)
        {
            fDryRun = true;
            return; // diff reset
        }
    }

    if(nBestHeight == 980950 || nBestHeight == 980951)
    {
        fDryRun = true;
        return;
    }

    // Test Fork
    if (nLiveForkToggle != 0) {
        // Do nothing
    }// TODO setup next testing fork

    // Standard, non-Dry Run
    fDryRun = false;
    return;
}

unsigned int VRX_Retarget(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    // Set base values
    bnVelocity = fProofOfStake ? Params().ProofOfStakeLimit() : Params().ProofOfWorkLimit();

    // Differentiate PoW/PoS prev block
    BlockVelocityType = GetLastBlockIndex(pindexLast, fProofOfStake);

    // Check for a dry run
    VRX_Dry_Run(pindexLast);
    if(fDryRun) { return bnVelocity.GetCompact(); }

    // Run VRX threadcurve
    VRX_ThreadCurve(pindexLast, fProofOfStake);
    if (fCRVreset) { return bnVelocity.GetCompact(); }

    // Retarget using simulation
    VRX_Simulate_Retarget();

    // Limit
    if (bnNew > bnVelocity) { bnNew = bnVelocity; }

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

    if (nHeight >= 980950) {
        nSubsidy = (500 * COIN);
    }

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

    if (pindexBest->nHeight >= 980950) {
        nSubsidy = (500 * COIN);
    }

    return nSubsidy + nFees;
}

//
// Xnode coin base reward
//
int64_t GetXNodePayment(int nHeight, int64_t blockValue)
{
    // No reward during alpha phase prototyping
    int64_t ret = 0 * COIN;
    return ret;
}
