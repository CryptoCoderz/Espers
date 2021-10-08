// Copyright (c) 2016-2021 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ESPERS_GENESIS_H
#define ESPERS_GENESIS_H


/** Genesis Start Time */
static const unsigned int timeGenesisBlock = 1473058800; // Mon, 05 Sep 2016 07:00:00 GMT
/** Genesis RegNet Start Time */
static const unsigned int timeRegNetGenesis = 1473059000; // Mon, 05 Sep 2016 07:00:00 GMT
/** Genesis Nonce */
static const unsigned int nNonceMain = 1934069;
/** Genesis Nonce Testnet */
static const unsigned int nNonceTest = 8903;
/** Genesis Nonce Regnet */
static const unsigned int nNonceReg = 8;
/** Main Net Genesis Block */
static const uint256 nGenesisBlock("0x000008d508d6e6577991a2f65fd974aea83c0644e6a0e1a3db047488e04398a3");
/** Test Net Genesis Block */
static const uint256 hashTestNetGenesisBlock("0x000072d91493eeb338abf5aa8d5c09db0af5f02d613d361d5f6a9906eca2aa74");
/** Reg Net Genesis Block */
static const uint256 hashRegNetGenesisBlock("0x511b6eeefb29d66cafae00a0746a8aa912bc13e1a79a7898f81a60ce3a79cea4");
/** Genesis Merkleroot */
static const uint256 nGenesisMerkle("0xe85e5d598a47643833dfb5732174813a866ab29888984ec07299d6a29865d2b1");

#endif
