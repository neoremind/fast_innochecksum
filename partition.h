// Use of this source code is governed by GNU General Public License
// that can be found in the License file.
//
// Author: neoremind (neoremind.com)

#ifndef FAST_INNOCHECKSUM_PARTITION_H_
#define FAST_INNOCHECKSUM_PARTITION_H_

#include <assert.h>
#include <string>

#include "partition_scan_cache_pool.h"
#include "util.h"

class Partition
{
public:
    Partition(uint32_t index, int dataFd, int dataDirectFd,
              uint64_t beginOffset, uint64_t endOffset,
              PartitionScanCachePool* scanCachePool)
        : mIndex(index),
          mDataFd(dataFd),
          mDataDirectFd(dataDirectFd),
          mBeginOffset(beginOffset),
          mEndOffset(endOffset),
          mScanCachePool(scanCachePool)
    {
        assert(beginOffset % UNIV_PAGE_SIZE == 0);
        assert(endOffset % UNIV_PAGE_SIZE == 0);
        mBeginPageNo = beginOffset / UNIV_PAGE_SIZE;
        mEndPageNo = endOffset / UNIV_PAGE_SIZE;
    }

    ~Partition() {}

    void Scan();

    void Prefetch();

    std::string ToString();

private:
    uint32_t mIndex;
    int mDataFd;
    int mDataDirectFd;
    uint32_t mBeginPageNo;
    uint32_t mEndPageNo;
    uint64_t mBeginOffset;
    uint64_t mEndOffset;
    PartitionScanCachePool* mScanCachePool;

    DISALLOW_COPY_AND_ASSIGN(Partition);
};

#endif  // FAST_INNOCHECKSUM_PARTITION_H_
