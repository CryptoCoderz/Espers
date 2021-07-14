// Copyright (c) 2016-2021 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ESPERS_CPHASHES_H
#define ESPERS_CPHASHES_H

#include "primitives/bignum.h"

static const uint256 CheckBlock_0001("0x0000068c30d404af5c95bc7caa3c464447e63ac3d6fc69d071b3965818487ad1");
static const uint256 CheckBlock_0002("0x00000932f50079a2bbe960fc72f0f5e65e779b735c2a1130ad4e16f57bdf9c46");
static const uint256 CheckBlock_0003("0x0000022f03fd526a65d7a0820905a2c74af56578b54150bab078d366b5156ab9");
static const uint256 CheckBlock_0004("0x000004765c62b759617faf01b8fa300e8668f5fa103af465ab73dc9de8d19247");
static const uint256 CheckBlock_0005("0x000000927b1cd9d59b387be8b64d6fb327ea949c8f5af966c6ce66bfb38f6143");
static const uint256 CheckBlock_0006("0x0000021a24fadf59b9156b0dd620ea5787f56c4a5c39a8f50a7d062504080c85");
static const uint256 CheckBlock_0007("0x00000af3a70c1e1a71c7ec8fb456f4f0a969e4ff3396f2f93636673956f8e0e8");
static const uint256 CheckBlock_0008("0x0000004fad7451598db014f0da0de71caafd6879ffa92131dab5b8b64a2a08b6");
static const uint256 CheckBlock_0009("0x00000571b840355ba518a846c4e9963e833c064efd6805fc7054659a8bb87445");
static const uint256 CheckBlock_0010("0x00000bb59e6e4541978f0489cec488e701a8f727fab158de94ac474d33bbe2de");
// Patch 6 additions
static const uint256 CheckBlock_0011("0x000000037681cfde759233a66ff9344bd02cb31eedcea1bea0e37e63af63bf9f");
static const uint256 CheckBlock_0012("0x5ddd13864d45098d9ab828244cef5d9fb9a2aed06ce59824fbc1cf039a6e0120");
static const uint256 CheckBlock_0013("0x00000013d904a17890d27114d9364e49bc8227a67249ead3290edc94c8e8558e");
static const uint256 CheckBlock_0014("0xb9bcdb6c31ffe8a380ea8be43ef50dbee5f24f1f161dece0888c52ce65dd4e2c");
static const uint256 CheckBlock_0015("0x00000005801537ce67cbb78349b6673e05a3835b50f97d336c6b0ce10862827b");
static const uint256 CheckBlock_0016("0x526e980f55e221879f9d67a8524f60077d9d834b950490f445265a82b7f13a23");
static const uint256 CheckBlock_0017("0xb95313efd4415c4e0f136337d82b1bfc8d303d7292ae6680bc39c1307fe6773c");
// Velocity additions
static const uint256 CheckBlock_0018("0x00000002736a3fb2074417e38cc02158bda9a0e216ff96718ad1203ae2a3cad2");
static const uint256 CheckBlock_0019("0x0000008dc88e24a0790041e551f0e36061c8d8ef6ec9907a3df4960b72ea9708");
static const uint256 CheckBlock_0020("0x000004938ad1637822be49315700e2082ec7f66d95f57aba977364855b89d397");
static const uint256 CheckBlock_0021("0x9a26fc52cdab2b863e4cebbfe2a8889209be938f9ab748f5ab257e39a9954b61");
// Swing Patch
static const uint256 CheckBlock_0022("0x45681988b0e9bc1275c0186160e913b13972083025f02dd35ce13a8d2835192c");
static const uint256 CheckBlock_0023("0x5ee5e4319108aca517f15574165887b97a7ce078e683b51d6d5decd0f74f3cd9");
// PoS-v3/Peer-Handling/Security upgrade
static const uint256 CheckBlock_0024("0x000003af3506ec5ad72b13de9d242432946797b0f495fd38b6d08ed15bdd7d45");
static const uint256 CheckBlock_0025("0x00000000030d685da8aae697b3d373b6a41b181d2f5c1e72b62ab9eb3d3aee01");
static const uint256 CheckBlock_0026("0x66d9ef5c9d3025516f7389cfa6bedbb0d0184f86b231d686f0ba87f8fd5d40ea");
static const uint256 CheckBlock_0027("0x0000000002d36834fed8602c4eee5fe439051673096197e0b0605d8d23642c5b");
// OpenSSL-1.1 Support/Security upgrade
static const uint256 CheckBlock_0028("0x00000000002c279a35355039733c81278b109a9aa9e6b943ade5345e4271f175");
static const uint256 CheckBlock_0029("0x24839e0efac44bc0fd76b2d64ebe04ac28a56feb7992a8cf63b44d104db62b79");
// Velocity Overhaul to v4
static const uint256 CheckBlock_0030("0x000000003cc3f2da309a6730a5d7ecd0359751ef33607955044735478a4d775c");

static const int64_t CheckHeight_0001 = 10;
static const int64_t CheckHeight_0002 = 25;
static const int64_t CheckHeight_0003 = 50;
static const int64_t CheckHeight_0004 = 75;
static const int64_t CheckHeight_0005 = 110;
static const int64_t CheckHeight_0006 = 140;
static const int64_t CheckHeight_0007 = 175;
static const int64_t CheckHeight_0008 = 225;
static const int64_t CheckHeight_0009 = 275;
static const int64_t CheckHeight_0010 = 356;
// Patch 6 additions
static const int64_t CheckHeight_0011 = 50000;
static const int64_t CheckHeight_0012 = 125000;
static const int64_t CheckHeight_0013 = 210000;
static const int64_t CheckHeight_0014 = 300000;
static const int64_t CheckHeight_0015 = 400000;
static const int64_t CheckHeight_0016 = 500000;
static const int64_t CheckHeight_0017 = 595000;
// Velocity additions
static const int64_t CheckHeight_0018 = 650000;
static const int64_t CheckHeight_0019 = 650035;
static const int64_t CheckHeight_0020 = 650147;
static const int64_t CheckHeight_0021 = 669750;
// Swing Patch
static const int64_t CheckHeight_0022 = 704190;
static const int64_t CheckHeight_0023 = 704195;
// PoS-v3/Peer-Handling/Security upgrade
static const int64_t CheckHeight_0024 = 725500;
static const int64_t CheckHeight_0025 = 746215;
static const int64_t CheckHeight_0026 = 767500;
static const int64_t CheckHeight_0027 = 769000;
// OpenSSL-1.1 Support/Security upgrade
static const int64_t CheckHeight_0028 = 777000;
static const int64_t CheckHeight_0029 = 786250;
// Velocity Overhaul to v4
static const int64_t CheckHeight_0030 = 982300;

#endif
