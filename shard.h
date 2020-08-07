// Use of this source code is governed by GNU General Public License
// that can be found in the License file.
//
// Author: neoremind (neoremind.com)

#ifndef FAST_INNOCHECKSUM_SHARD_H_
#define FAST_INNOCHECKSUM_SHARD_H_

#include <string>
#include <vector>

#include "partition.h"
#include "util.h"

class Shard
{
public:
    static RetCode Open(uint32_t shardIndex, int dataFd, int dataDirectFd,
                        uint64_t beginOffset, uint64_t endOffset,
                        uint32_t numOfScanCachePartition,
                        uint64_t partitionByteSize, Shard** eptr);

    explicit Shard(uint32_t shardIndex, int dataFd, int dataDirectFd,
                   uint64_t beginOffset, uint64_t endOffset,
                   uint32_t numOfScanCachePartition,
                   uint64_t partitionByteSize);

    ~Shard();

    void Scan();

    std::string ToString();

private:
    RetCode Init();

    uint32_t mShardIndex;
    int mDataFd;
    int mDataDirectFd;
    uint64_t mBeginOffset;
    uint64_t mEndOffset;
    uint32_t mNumOfScanCachePartition;
    uint64_t mPartitionByteSize;
    std::vector<Partition*> mPartitions;
    PartitionScanCachePool* mPartitionScanCachePool;
};

class PrefetchScanTask
{
public:
    PrefetchScanTask(const std::vector<Partition*>& mPartitions)
        : mPartitions(mPartitions)
    {
    }

    void operator()() const
    {
        for (uint32_t i = 0; i < mPartitions.size(); ++i)
        {
            mPartitions[i]->Prefetch();
        }
    }

private:
    std::vector<Partition*> mPartitions;
};

#endif  // FAST_INNOCHECKSUM_SHARD_H_