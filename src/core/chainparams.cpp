// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2016-2021 The Espers developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "assert.h"

#include "chainparams.h"
#include "main.h"
#include "util/util.h"

#include <boost/assign/list_of.hpp>

using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

//
// Main network
//

// Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress> &vSeedsOut, const SeedSpec6 *data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7*24*60*60;
    for (unsigned int i = 0; i < count; i++)
    {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

class CMainParams : public CChainParams {
public:
    CMainParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0x4e;
        pchMessageStart[1] = 0xaa;
        pchMessageStart[2] = 0x32;
        pchMessageStart[3] = 0x1c;
        /** MainNet Parameters */
        vAlertPubKey = ParseHex("04e22531e96c9056be6b659c91a94fbfebeb5d5257fe044b88695c62f7c2f81f85d131a669df3be611393f454852a2d08c6314bba5ca3cbe5616262da3d4a6efac");
        nDefaultPort = 22448;
        nRPCPort = 22442;
        bnProofOfWorkLimit = ~uint256(0) >> 20;
        bnProofOfStakeLimit = ~uint256(0) >> 20;
        // Genesis Message
        const char* pszTimestamp = "Krypton Abandons Ethereum for Bitcoin Proof of Stake Blockchain after 51% Attack : Mon, 05 Sep 2016 07:00:00 GMT";
        std::vector<CTxIn> vin;
        vin.resize(1);
        vin[0].scriptSig = CScript() << 0 << CScriptNum(42) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        std::vector<CTxOut> vout;
        vout.resize(1);
        vout[0].SetEmpty();
        CTransaction txNew(1, timeGenesisBlock, vin, vout, 0);
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime    = timeGenesisBlock;
        genesis.nBits    = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce   = nNonceMain;
        /** Genesis Block MainNet */
        /*
        Hashed MainNet Genesis Block Output
        block.hashMerkleRoot == e85e5d598a47643833dfb5732174813a866ab29888984ec07299d6a29865d2b1
        block.nTime = 1473058800
        block.nNonce = 1934069
        block.GetHash = 000008d508d6e6577991a2f65fd974aea83c0644e6a0e1a3db047488e04398a3
        */
        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == nGenesisBlock);
        assert(genesis.hashMerkleRoot == nGenesisMerkle);
        // Base58 Prefix Assigning for Address start letters
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,33);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,92);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,144);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();
        // Espers dns seeds
        vSeeds.push_back(CDNSSeedData("Alias",  "0.0.0.0"));
        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));
        // Initial block spacing, kept for syncing only.
        nTargetSpacing = 1 * 40;
        if(nBestHeight > nBlocktimeregress) { nTargetSpacing = 2.5 * 60; }
        if(nBestHeight > nGravityFork) { nTargetSpacing = BLOCK_SPACING * 1; }
        nTargetTimespan = 10 * nTargetSpacing;
        // Delay PoS start until - swap start
        nStartPoSBlock = 2125;
    }

    virtual const CBlock& GenesisBlock() const { return genesis; }
    virtual Network NetworkID() const { return CChainParams::MAIN; }

    virtual const vector<CAddress>& FixedSeeds() const {
        return vFixedSeeds;
    }
protected:
    CBlock genesis;
    vector<CAddress> vFixedSeeds;
};
static CMainParams mainParams;


//
// Testnet
//

class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0x4c;
        pchMessageStart[1] = 0xe6;
        pchMessageStart[2] = 0x68;
        pchMessageStart[3] = 0x5a;
        bnProofOfWorkLimit = ~uint256(0) >> 16;
        bnProofOfStakeLimit = ~uint256(0) >> 16;
        /** TestNet Parameters */
        vAlertPubKey = ParseHex("02e22531e96c9056be6b659c91a94fbfebeb5d5257fe044b88695c62f7c2f81f85d131a669df3be611393f454852a2d08c6314bba5ca3cbe5616262db3d4a6efac");
        nDefaultPort = 32448;
        nRPCPort = 32440;
        strDataDir = "testnet";
        // Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nBits  = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce = nNonceTest;
        /** Genesis Block TestNet */
        /*
        Hashed TestNet Genesis Block Output
        block.nTime = 1473058800
        block.nNonce = 8903
        block.GetHash = 000072d91493eeb338abf5aa8d5c09db0af5f02d613d361d5f6a9906eca2aa74
        */
        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == hashTestNetGenesisBlock);
        // TestNet has no seeds
        vFixedSeeds.clear();
        vSeeds.clear();
        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));
        // Base58 Prefix Assigning for Address start letters
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,34);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,94);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,143);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();
        // TestNet uses latest VRX retarget
        nTargetSpacing = 0;
        nStartPoSBlock = 0;
    }
    virtual Network NetworkID() const { return CChainParams::TESTNET; }
};
static CTestNetParams testNetParams;


//
// Regression test
//
class CRegTestParams : public CTestNetParams {
public:
    CRegTestParams() {
        pchMessageStart[0] = 0x2b;
        pchMessageStart[1] = 0xde;
        pchMessageStart[2] = 0x09;
        pchMessageStart[3] = 0x1a;
        /** RegNet Parameters */
        bnProofOfWorkLimit = ~uint256(0) >> 1;
        genesis.nTime = timeRegNetGenesis;
        genesis.nBits  = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce = nNonceReg;
        hashGenesisBlock = genesis.GetHash();
        nDefaultPort = 32445;
        strDataDir = "regtest";
        /** Genesis Block TestNet */
        /*
        Hashed RegNet Genesis Block Output
        block.hashMerkleRoot == e85e5d598a47643833dfb5732174813a866ab29888984ec07299d6a29865d2b1
        block.nTime = 1473059000
        block.nNonce = 8
        block.GetHash = 511b6eeefb29d66cafae00a0746a8aa912bc13e1a79a7898f81a60ce3a79cea4
        */
        assert(hashGenesisBlock == hashRegNetGenesisBlock);
        // Regtest mode doesn't have any DNS seeds.
        vSeeds.clear();
    }

    virtual bool RequireRPCPassword() const { return false; }
    virtual Network NetworkID() const { return CChainParams::REGTEST; }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = &mainParams;

const CChainParams &Params() {
    return *pCurrentParams;
}

void SelectParams(CChainParams::Network network) {
    switch (network) {
        case CChainParams::MAIN:
            pCurrentParams = &mainParams;
            break;
        case CChainParams::TESTNET:
            pCurrentParams = &testNetParams;
            break;
        case CChainParams::REGTEST:
            pCurrentParams = &regTestParams;
            break;
        default:
            assert(false && "Unimplemented network");
            return;
    }
}

bool SelectParamsFromCommandLine() {
    bool fRegTest = GetBoolArg("-regtest", false);
    bool fTestNet = GetBoolArg("-testnet", false);

    if (fTestNet && fRegTest) {
        return false;
    }

    if (fRegTest) {
        SelectParams(CChainParams::REGTEST);
    } else if (fTestNet) {
        SelectParams(CChainParams::TESTNET);
    } else {
        SelectParams(CChainParams::MAIN);
    }
    return true;
}
