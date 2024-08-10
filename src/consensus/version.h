// Copyright (c) 2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ESPERS_VERSION_H
#define ESPERS_VERSION_H

#include "clientversion.h"
#include <string>
#include <stdint.h>

//
// client versioning
//

static const int CLIENT_VERSION =
                           1000000 * CLIENT_VERSION_MAJOR
                         +   10000 * CLIENT_VERSION_MINOR
                         +     100 * CLIENT_VERSION_REVISION
                         +       1 * CLIENT_VERSION_BUILD;

extern const std::string CLIENT_NAME;
extern const std::string CLIENT_BUILD;
extern const std::string CLIENT_DATE;

//
// database format versioning
//
static const int DATABASE_VERSION = 70509;

//
// network protocol versioning
//
static const int PROTOCOL_VERSION = 60049;

// intial proto version, to be increased after version/verack negotiation
static const int INIT_PROTO_VERSION = 209;

// disconnect from peers older than this proto version
static const int MIN_PEER_PROTO_VERSION = 60044;

// nTime field added to CAddress, starting with this version;
// if possible, avoid requesting addresses nodes older than this
static const int CADDR_TIME_VERSION = 31402;

// only request blocks from nodes outside this range of versions
static const int NOBLKS_VERSION_START = 0;
static const int NOBLKS_VERSION_END = 60043;

// hard cutoff time for legacy network connections
static const int64_t HRD_LEGACY_CUTOFF = 1723506989; // ON (TOGGLED Mon, 12 August 2024 23:56:29 GMT)

// hard cutoff time for future network connections
static const int64_t HRD_FUTURE_CUTOFF = 9993058800; // OFF (NOT TOGGLED)

// BIP 0031, pong message, is enabled for all versions AFTER this one
static const int BIP0031_VERSION = 60000;

// "mempool" command, enhanced "getdata" behavior starts with this version:
static const int MEMPOOL_GD_VERSION = 60002;

// "demi-nodes" command, enhanced "getdata" behavior starts with this version:
static const int DEMINODE_VERSION = 60039;

// minimum peer version accepted by PAS (Pubkey Alias System)
static const int MIN_PASERVICE_PROTO_VERSION = 62042;

// reject blocks with non-canonical signatures starting from this version
static const int CANONICAL_BLOCK_SIG_VERSION = 60045;
static const int CANONICAL_BLOCK_SIG_LOW_S_VERSION = 60046;

// minimum peer version that can receive xnode payments
// V1 - Last protocol version before update
// V2 - Newest protocol version
static const int MIN_XNODE_PAYMENT_PROTO_VERSION_1 = 60036;
static const int MIN_XNODE_PAYMENT_PROTO_VERSION_2 = 60036;

#endif
