// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: neoremind (neoremind.com)

#include "scanner.h"

#include <fcntl.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "thread_pool.h"
#include "util.h"

Scanner::Scanner(std::string filePath, unsigned long long int fileSize,
                 uint64_t shardByteSize, uint32_t numOfScanCachePartition,
                 uint64_t partitionByteSize, uint32_t numOfConcurrentShard)
    : mFilePath(filePath),
      mFileSize(fileSize),
      mShardByteSize(shardByteSize),
      mNumOfScanCachePartition(numOfScanCachePartition),
      mPartitionByteSize(partitionByteSize),
      mNumOfConcurrentShard(numOfConcurrentShard)
{
    printf("open %s\n", filePath.c_str());
    mDataFd = ::open(mFilePath.c_str(), O_NOATIME | O_RDWR, 0644);
    mDataDirectFd =
        ::open(mFilePath.c_str(), O_DIRECT | O_NOATIME | O_RDWR, 0644);
}

Scanner::~Scanner() {}

void Scanner::Execute()
{
    unsigned long long int numOfShard = mFileSize / mShardByteSize;
    if (numOfShard == 0)
    {
        Shard* shard;
        Shard::Open(0, mDataFd, mDataDirectFd, 0, mFileSize,
                    mNumOfScanCachePartition, mPartitionByteSize, &shard);
        shard->Scan();
        delete shard;
    }
    else
    {
        ThreadPool pool(mNumOfConcurrentShard);
        std::vector<std::future<RetCode>> results;

        uint64_t beginOffset = 0;
        uint64_t endOffset = mShardByteSize;
        for (size_t i = 0; i < numOfShard; i++)
        {
            results.emplace_back(pool.enqueue(
                [](uint32_t shardIndex, int dataFd, int dataDirectFd,
                   uint64_t beginOffset, uint64_t endOffset,
                   uint32_t numOfScanCachePartition,
                   uint64_t partitionByteSize) {
                    Shard* shard;
                    Shard::Open(shardIndex, dataFd, dataDirectFd, beginOffset,
                                endOffset, numOfScanCachePartition,
                                partitionByteSize, &shard);
                    shard->Scan();
                    delete shard;
                    return kSucc;
                },
                i, mDataFd, mDataDirectFd, beginOffset, endOffset,
                mNumOfScanCachePartition, mPartitionByteSize));
            beginOffset += mShardByteSize;
            endOffset += mShardByteSize;
        }
        unsigned long long int left = mFileSize % mShardByteSize;
        if (left != 0)
        {
            results.emplace_back(pool.enqueue(
                [](uint32_t shardIndex, int dataFd, int dataDirectFd,
                   uint64_t beginOffset, uint64_t endOffset,
                   uint32_t numOfScanCachePartition,
                   uint64_t partitionByteSize) {
                    Shard* shard;
                    Shard::Open(shardIndex, dataFd, dataDirectFd, beginOffset,
                                endOffset, numOfScanCachePartition,
                                partitionByteSize, &shard);
                    shard->Scan();
                    delete shard;
                    return kSucc;
                },
                numOfShard, mDataFd, mDataDirectFd, beginOffset,
                beginOffset + left, mNumOfScanCachePartition,
                mPartitionByteSize));
        }

        for (auto&& result : results)
        {
            if (result.get() != kSucc)
            {
                printf("scanner fails");
                exit(1);
            }
        }
    }
}