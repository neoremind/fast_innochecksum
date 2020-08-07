// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: neoremind (neoremind.com)

#ifndef FAST_INNOCHECKSUM_SCANNER_H_
#define FAST_INNOCHECKSUM_SCANNER_H_

#include <string>

#include "shard.h"

class Scanner
{
public:
    explicit Scanner(std::string filePath, unsigned long long int fileSize,
                     uint64_t shardByteSize, uint32_t numOfScanCachePartition,
                     uint64_t partitionByteSize, uint32_t numOfConcurrentShard);

    ~Scanner();

    void Execute();

private:
    std::string mFilePath;
    unsigned long long int mFileSize;
    int mDataFd;
    int mDataDirectFd;
    uint64_t mShardByteSize;
    uint32_t mNumOfScanCachePartition;
    uint64_t mPartitionByteSize;
    uint32_t mNumOfConcurrentShard;
};

#endif  // FAST_INNOCHECKSUM_SCANNER_H_