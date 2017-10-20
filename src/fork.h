// Copyright (c) 2016-2017 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ESPERS_FORK_H
#define ESPERS_FORK_H

#include "bignum.h"

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
/** Velocity toggle block */
static const int64_t VELOCITY_TOGGLE = 650000; // Implementation of the Velocity system into the chain.
/** Velocity retarget toggle block */
static const int64_t VELOCITY_TDIFF = 667350; // Use Velocity's retargetting method.

#endif
