// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "blocksizecalculator.h"

using namespace BlockSizeCalculator;
using namespace std;

static std::vector<unsigned int> blocksizes;
static bool sorted = false;

unsigned int BlockSizeCalculator::ComputeBlockSize(CBlockIndex *pblockindex, unsigned int pastblocks) {

	unsigned int proposedMaxBlockSize = 0;
    unsigned int result = MIN_BLOCK_SIZE;

	LOCK(cs_main);

	proposedMaxBlockSize = ::GetMedianBlockSize(pblockindex, pastblocks);

	if (proposedMaxBlockSize > 0) {
		//Absolute max block size will be 2^32-1 bytes due to the fact that unsigned int's are 4 bytes
		result = proposedMaxBlockSize * MAX_BLOCK_SIZE_INCREASE_MULTIPLE;
		result = result < proposedMaxBlockSize ?
				std::numeric_limits<unsigned int>::max() :
				result;
        if (result < MIN_BLOCK_SIZE) {
            result = MIN_BLOCK_SIZE;
		}
	}

	return result;

}

inline unsigned int BlockSizeCalculator::GetMedianBlockSize(
		CBlockIndex *pblockindex, unsigned int pastblocks) {

	blocksizes = ::GetBlockSizes(pblockindex, pastblocks);

	if(!sorted) {
		std::sort(blocksizes.begin(), blocksizes.end());
		sorted = true;
	}

	unsigned int vsize = blocksizes.size();
	if (vsize == pastblocks) {
		double median = 0;
		if ((vsize % 2) == 0) {
			median = (blocksizes[vsize / 2] + blocksizes[(vsize / 2) - 1]) / 2.0;
		} else {
			median = blocksizes[vsize / 2];
		}
		return static_cast<unsigned int>(floor(median));
	} else {
		return 0;
	}

}

inline std::vector<unsigned int> BlockSizeCalculator::GetBlockSizes(
		CBlockIndex *pblockindex, unsigned int pastblocks) {

	if (pblockindex->nHeight < (int)pastblocks) {
		return blocksizes;
	}

	int firstBlock = pblockindex->nHeight - pastblocks;

	if (blocksizes.size() > 0) {

		int latestBlockSize = ::GetBlockSize(pblockindex);

		if (latestBlockSize != -1) {

			CBlockIndex *firstBlockIndex = FindBlockByHeight(firstBlock);
			int oldestBlockSize = ::GetBlockSize(firstBlockIndex);
			if (oldestBlockSize != -1) {
				std::vector<unsigned int>::iterator it;
				it = std::find(blocksizes.begin(), blocksizes.end(),
						oldestBlockSize);
				if (it != blocksizes.end()) {
					blocksizes.erase(it);
					it = std::lower_bound(blocksizes.begin(), blocksizes.end(),
							latestBlockSize);
					blocksizes.insert(it, latestBlockSize);
				}
			}

		}

	} else {

		while (pblockindex != NULL && pblockindex->nHeight > firstBlock) {

			int blocksize = ::GetBlockSize(pblockindex);
			if (blocksize != -1) {

				blocksizes.push_back(blocksize);
			}

			pblockindex = pblockindex->pprev;
		}

	}

	return blocksizes;

}

inline int BlockSizeCalculator::GetBlockSize(CBlockIndex *pblockindex) {

	if (pblockindex == NULL) {
		return -1;
	}

	const CDiskBlockPos& pos = pblockindex->GetBlockPos();

	CAutoFile filein(OpenBlockFile(pos, false), SER_DISK, CLIENT_VERSION);
	FILE* blockFile = filein.release();
	long int filePos = ftell(blockFile);
	fseek(blockFile, filePos - sizeof(uint32_t), SEEK_SET);

	uint32_t size = 0;
	fread(&size, sizeof(uint32_t), 1, blockFile);
	fclose(blockFile);
	return (unsigned int) size;

}
