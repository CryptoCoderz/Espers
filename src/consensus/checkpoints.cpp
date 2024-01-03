// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2016-2024 The Espers developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"
#include "cphashes.h"
#include "database/txdb.h"
#include "core/main.h"
#include "primitives/uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    //
    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    //
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        ( 0,                      nGenesisBlock )
        ( CheckHeight_0001,       CheckBlock_0001)
        ( CheckHeight_0002,       CheckBlock_0002)
        ( CheckHeight_0003,       CheckBlock_0003)
        ( CheckHeight_0004,       CheckBlock_0004)
        ( CheckHeight_0005,       CheckBlock_0005)
        ( CheckHeight_0006,       CheckBlock_0006)
        ( CheckHeight_0007,       CheckBlock_0007)
        ( CheckHeight_0008,       CheckBlock_0008)
        ( CheckHeight_0009,       CheckBlock_0009)
        ( CheckHeight_0010,       CheckBlock_0010)
        ( CheckHeight_0011,       CheckBlock_0011)
        ( CheckHeight_0012,       CheckBlock_0012)
        ( CheckHeight_0013,       CheckBlock_0013)
        ( CheckHeight_0014,       CheckBlock_0014)
        ( CheckHeight_0015,       CheckBlock_0015)
        ( CheckHeight_0016,       CheckBlock_0016)
        ( CheckHeight_0017,       CheckBlock_0017)
        ( CheckHeight_0018,       CheckBlock_0018)
        ( CheckHeight_0019,       CheckBlock_0019)
        ( CheckHeight_0020,       CheckBlock_0020)
        ( 667351,                 uint256("0xad798d349cc8e09e66f12c4409ac996465906fbb28e8ecf4e25602d0e8037b19"))
        ( CheckHeight_0021,       CheckBlock_0021)
        ( CheckHeight_0022,       CheckBlock_0022)
        ( CheckHeight_0023,       CheckBlock_0023)
        ( CheckHeight_0024,       CheckBlock_0024)
        ( CheckHeight_0025,       CheckBlock_0025)
        ( CheckHeight_0026,       CheckBlock_0026)
        ( CheckHeight_0027,       CheckBlock_0027)
        ( CheckHeight_0028,       CheckBlock_0028)
        ( CheckHeight_0029,       CheckBlock_0029)
        ( CheckHeight_0030,       CheckBlock_0030)
        (1024000,                 uint256("0x618791c1da28b1c6130bcf8f79f50010b7ce2f511885b769794255651b5920b9"))
        (1134000,                 uint256("0x0000000300883daacce5afc680baf2f4c13ba11c7f5d4fc363f14dabe8490204"))
    ;

    // TestNet has no checkpoints
    static MapCheckpoints mapCheckpointsTestnet;

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    int GetTotalBlocksEstimate()
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        if (checkpoints.empty())
            return 0;
        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    // Automatically select a suitable sync-checkpoint
    const CBlockIndex* AutoSelectSyncCheckpoint()
    {
        const CBlockIndex *pindex = pindexBest;
        // Search backward for a block within max span and maturity window
        // Taking into account our 120 block depth + reorganize depth
        while (pindex->pprev && pindex->nHeight + (BLOCK_TEMP_CHECKPOINT_DEPTH + 500) > pindexBest->nHeight)
            pindex = pindex->pprev;
        return pindex;
    }

    // Check against synchronized checkpoint
    bool CheckSync(int nHeight)
    {
        const CBlockIndex* pindexSync = AutoSelectSyncCheckpoint();
        if (nHeight <= pindexSync->nHeight){
            return false;
        }
        return true;
    }
}
