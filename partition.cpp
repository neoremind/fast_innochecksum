// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: neoremind (neoremind.com)

#include "partition.h"

#include <fcntl.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>

#include "util.h"

void Partition::Prefetch()
{
    ScanCacheItem* item = nullptr;
    while (true)
    {
        item = mScanCachePool->Get(mIndex);
        if (item != nullptr)
        {
            break;
        }
        usleep(1);
    }
    // printf("prefetch %d start\n", mIndex);
#if USE_DIRECT_IO == 1
    pread64(mDataDirectFd, static_cast<char*>(item->GetData()),
            mEndOffset - mBeginOffset, mBeginOffset);
#else
    pread64(mDataFd, static_cast<char*>(item->GetData()),
            mEndOffset - mBeginOffset, mBeginOffset);
#endif
    // printf("prefetch %d done\n", mIndex);
    item->SetDataReady();
    item->Release();
}

void Partition::Scan()
{
    ScanCacheItem* item = nullptr;
    // printf("scan %d start\n", mIndex);
    while (true)
    {
        item = mScanCachePool->Get(mIndex);
        if (item != nullptr)
        {
            break;
        }
        usleep(1);
    }
    item->WaitDataReady();
    uchar* data = static_cast<uchar*>(item->GetData());
    for (uint32_t i = mBeginPageNo; i < mEndPageNo; i++)
    {
        ulint lsn = mach_read_from_4(data + FIL_PAGE_LSN + 4);
        ulint lsn2 = mach_read_from_4(data + UNIV_PAGE_SIZE -
                                      FIL_PAGE_END_LSN_OLD_CHKSUM + 4);
#if LOG_LEVEL == LOG_LEVEL_TRACE
        printf("page %d: lsn: first = %lu; second = %lu\n", i, lsn, lsn2);
#endif
        if (unlikely(lsn != lsn2))
        {
            fprintf(stderr,
                    "page %d invalid (fails log sequence number check)\n", i);
#if SKIP_ERROR == 0
            exit(1);
#endif
        }

        ulint checksum = buf_calc_page_crc32(data);
        ulint checksum2 = mach_read_from_4(data + FIL_PAGE_SPACE_OR_CHKSUM);
#if LOG_LEVEL == LOG_LEVEL_TRACE
        printf("page %d: calculated = %lu; recorded = %lu\n", i, checksum,
               checksum2);
#endif
        if (unlikely(checksum2 != 0 && checksum != checksum2))
        {
            fprintf(stderr, "page %d invalid (fails new style checksum)\n", i);
#if SKIP_ERROR == 0
            exit(1);
#endif
        }

        data += UNIV_PAGE_SIZE;
    }

    // printf("scan %d done\n", mIndex);
    item->Release();
    item = nullptr;
}

std::string Partition::ToString()
{
    std::string result("partition #");
    result.append(std::to_string(mIndex));
    result.append(", [");
    result.append(std::to_string(mBeginPageNo));
    result.append(",");
    result.append(std::to_string(mEndPageNo));
    result.append("] mBeginOffset:");
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