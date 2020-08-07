// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: neoremind (neoremind.com), Xiaolu Zhang

#ifndef FAST_INNOCHECKSUM_PARTITION_SCAN_CACHE_POOL_H_
#define FAST_INNOCHECKSUM_PARTITION_SCAN_CACHE_POOL_H_

#include <mutex>
#include <stdint.h>
#include <condition_variable>

#include "util.h"

class PartitionScanCachePool;

class ScanCacheItem
{
public:
    ScanCacheItem();

    ~ScanCacheItem();

    void WaitDataReady();

    void SetDataReady();

    void* GetData() { return mData; }

    void Release();

private:
    friend class PartitionScanCachePool;

    uint32_t mPartitionIndex;
    void* mData;
    std::mutex mMutex;
    std::condition_variable mCond;
    uint32_t mRefCount;
    bool mCompleted;

    DISALLOW_COPY_AND_ASSIGN(ScanCacheItem);
};

class PartitionScanCachePool
{
public:
    PartitionScanCachePool(uint32_t capacity, uint32_t dataSize);

    ~PartitionScanCachePool();

    ScanCacheItem* Get(uint32_t partitionIndex);

private:
    uint32_t mCapacity;
    uint32_t mDataSize;
    ScanCacheItem* mItems;

    DISALLOW_COPY_AND_ASSIGN(PartitionScanCachePool);
};

#endif  // FAST_INNOCHECKSUM_PARTITION_SCAN_CACHE_POOL_H_
