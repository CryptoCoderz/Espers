// Copyright (c) 2014 The Cryptocoin Revival Foundation
// Copyright (c) 2015-2024 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VELOCITY_H
#define VELOCITY_H 1

#include "core/blockparams.h"
#include "core/main.h"

class CBlock;
class CBlockIndex;

// Velocity Parameters
static const          int VELOCITY_HEIGHT[]    = { VELOCITY_TOGGLE }; /** Height to start Velocity */
static const          int VELOCITY_TERMINAL[]  = { VELOCITY_TDIFF }; /** Height to start Velocity retargetting */
static const          int VELOCITY_MAX_RATE[]  = { BLOCK_SPACING_MAX }; /** Rate to Velocity in seconds */
static const          int VELOCITY_RATE[]      = { BLOCK_SPACING }; /** Rate to Velocity in seconds */
static const          int VELOCITY_MIN_RATE[]  = { BLOCK_SPACING_MIN }; /** Rate to Velocity in seconds */
static const unsigned int VELOCITY_MIN_TX[]    = { MIN_TX_COUNT }; /** Minimum amount (not value of!) of TX in a block to bypass Velocity-Rate */
static const          int VELOCITY_MIN_VALUE[] = { MIN_TX_VALUE }; /** Minimum value of the TX in a block to bypass Velocity-Rate (without COIN base) */
static const          int64_t VELOCITY_MIN_FEE[]   = { MIN_TX_FEE }; /** Minimum value of accumulated fees of the TX in a block to bypass Velocity-Rate (without COIN base) */
static const          bool VELOCITY_EXPLICIT[]  = { false }; /** Require all switches to trigger a block */

// Value set 1
bool Velocity_check(int nHeight);
bool Velocity(CBlockIndex* prevBlock, CBlock* block, bool fFactor_tx);
bool tx_Factor(CBlockIndex* prevBlock, CBlock* block);
bool bIndex_Factor(CBlockIndex* InSplitPoint, CBlockIndex* InSplitEnd);

int VelocityI(int nHeight);
bool RollingCheckpoints(int nHeight, CBlockIndex *pindexRequest);

extern bool VELOCITY_FACTOR; /** Treat Switches as factors of Block Scanning */
extern uint256 RollingBlock;
extern int64_t RollingHeight;

#endif
