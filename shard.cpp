// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: neoremind (neoremind.com)

#include "shard.h"

#include <string>
#include <thread>
#include <vector>

Shard::Shard(uint32_t shardIndex, int dataFd, int dataDirectFd,
             uint64_t beginOffset, uint64_t endOffset,
             uint32_t numOfScanCachePartition, uint64_t partitionByteSize)
    : mShardIndex(shardIndex),
      mDataFd(dataFd),
      mDataDirectFd(dataDirectFd),
      mBeginOffset(beginOffset),
      mEndOffset(endOffset),
      mNumOfScanCachePartition(numOfScanCachePartition),
      mPartitionByteSize(partitionByteSize)
{
}

RetCode Shard::Init()
{
    mPartitionScanCachePool = new PartitionScanCachePool(
        mNumOfScanCachePartition, mPartitionByteSize);
    uint64_t numOfPartition = (mEndOffset - mBeginOffset) / mPartitionByteSize;
    uint64_t left = (mEndOffset - mBeginOffset) % mPartitionByteSize;
    uint64_t beginOffset = mBeginOffset;
    for (size_t i = 0; i < numOfPartition; i++)
    {
        Partition* partition = new Partition(
            i, mDataFd, mDataDirectFd, beginOffset,
            beginOffset + mPartitionByteSize, mPartitionScanCachePool);
        mPartitions.push_back(partition);
        beginOffset += mPartitionByteSize;
    }
    if (left != 0)
    {
        Partition* partition = new Partition(
            static_cast<uint32_t>(numOfPartition), mDataFd, mDataDirectFd,
            beginOffset, beginOffset + left, mPartitionScanCachePool);
        mPartitions.push_back(partition);
    }
    return kSucc;
}

RetCode Shard::Open(uint32_t shardIndex, int dataFd, int dataDirectFd,
                    uint64_t beginOffset, uint64_t endOffset,
                    uint32_t numOfScanCachePartition,
                    uint64_t partitionByteSize, Shard** sptr)
{
    *sptr = NULL;
    Shard* shard =
        new Shard(shardIndex, dataFd, dataDirectFd, beginOffset, endOffset,
                  numOfScanCachePartition, partitionByteSize);
    RetCode ret = shard->Init();
    if (ret != kSucc)
    {
        delete shard;
        shard = NULL;
    }
    else
    {
        *sptr = shard;
    }
    return ret;
}

Shard::~Shard()
{
    for (auto iter = mPartitions.begin(); iter != mPartitions.end(); ++iter)
    {
        delete *iter;
    }
    mPartitions.clear();
    delete mPartitionScanCachePool;
}

void Shard::Scan()
{
#if LOG_LEVEL == LOG_LEVEL_DEBUG
    printf("%s start\n", this->ToString().c_str());
#endif
    PrefetchScanTask task(mPartitions);
    std::thread prefetchThread(task);
    for (uint32_t i = 0; i < mPartitions.size(); ++i)
    {
        mPartitions[i]->Scan();
    }
    prefetchThread.join();
#if LOG_LEVEL == LOG_LEVEL_DEBUG
    printf("%s done\n", this->ToString().c_str());
    for (size_t i = 0; i < mPartitions.size(); i++)
    {
        printf("%s done\n", mPartitions[i]->ToString().c_str());
    }
#endif
}

std::string Shard::ToString()
{
    std::string result("shard #");
    result.append(std::to_string(mShardIndex));
    result.append(" mBeginOffset:");
    result.append(std::to_string(mBeginOffset));
    result.append(" (");
    result.append(std::to_string(mBeginOffset / 1024 / 1024));
    result.append("MB) mEndOffset:");
    result.append(std::to_string(mEndOffset));
    result.append(" (");
    result.append(std::to_string(mEndOffset / 1024 / 1024));
    result.append("MB) ");
    return result;
}