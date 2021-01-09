// Copyright (c) 2016-2021 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ESPERS_BLOCKPARAMS_H
#define ESPERS_BLOCKPARAMS_H

#include "node/net.h"
#include "chain.h"
#include "primitives/bignum.h"

// Define difficulty retarget algorithms
enum DiffMode {
    DIFF_DEFAULT = 0, // Default to invalid 0
    DIFF_PPC     = 1, // Retarget using Peercoin per-block
    DIFF_DGW     = 2, // Retarget using DarkGravityWave v3
    DIFF_VRX     = 3, // Retarget using Terminal-Velocity-RateX
};

void VRXswngdebug();
void VRXdebug();
void GNTdebug();
void VRX_BaseEngine(const CBlockIndex* pindexLast, bool fProofOfStake);
void VRX_ThreadCurve(const CBlockIndex* pindexLast, bool fProofOfStake);
void VRX_Dry_Run(const CBlockIndex* pindexLast);
void VRX_Simulate_Retarget();
unsigned int PeercoinDiff(const CBlockIndex* pindexLast, bool fProofOfStake);
unsigned int DarkGravityWave(const CBlockIndex* pindexLast, bool fProofOfStake);
unsigned int VRX_Retarget(const CBlockIndex* pindexLast, bool fProofOfStake);
unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake);
int64_t GetProofOfWorkReward(int64_t nHeight, int64_t nFees);
int64_t GetProofOfStakeReward(int64_t nCoinAge, int64_t nFees);


#endif // ESPERS_BLOCKPARAMS_H
