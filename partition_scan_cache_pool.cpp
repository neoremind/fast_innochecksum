// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: neoremind (neoremind.com), Xiaolu Zhang

#include "partition_scan_cache_pool.h"

#include <iostream>
#include <stdio.h>
#include <unistd.h>

ScanCacheItem::ScanCacheItem()
    : mPartitionIndex(-1), mData(nullptr), mRefCount(0), mCompleted(false)
{
}

ScanCacheItem::~ScanCacheItem()
{
    if (mData != nullptr)
    {
        free(mData);
        mData = nullptr;
    }
}

void ScanCacheItem::WaitDataReady()
{
    std::unique_lock<std::mutex> guard(mMutex);
    while (!mCompleted)
    {
        mCond.wait(guard);
    }
}

void ScanCacheItem::SetDataReady()
{
    std::lock_guard<std::mutex> guard(mMutex);
    mCompleted = true;
    mCond.notify_all();
}

void ScanCacheItem::Release()
{
    std::lock_guard<std::mutex> guard(mMutex);
    mRefCount--;
}

PartitionScanCachePool::PartitionScanCachePool(uint32_t capacity,
                                               uint32_t dataSize)
    : mCapacity(capacity), mDataSize(dataSize)
{
    mItems = new ScanCacheItem[mCapacity];
}

PartitionScanCachePool::~PartitionScanCachePool()
{
    delete[] mItems;
    mItems = nullptr;
}

ScanCacheItem* PartitionScanCachePool::Get(uint32_t partitionIndex)
{
    uint32_t index = partitionIndex % mCapacity;
    std::lock_guard<std::mutex> guard(mItems[index].mMutex);
    if (mItems[index].mPartitionIndex == partitionIndex)
    {
        mItems[index].mRefCount++;
        return &mItems[index];
    }
    if (mItems[index].mRefCount > 0)
    {
        return nullptr;
    }
    mItems[index].mPartitionIndex = partitionIndex;
    mItems[index].mCompleted = false;
    mItems[index].mRefCount = 1;
    if (mItems[index].mData == nullptr)
    {
        posix_memalign(&mItems[index].mData, getpagesize(), mDataSize);
    }
    return &mItems[index];
}
