// Use of this source code is governed by GNU General Public License
// that can be found in the License file.
//
// Author: neoremind (neoremind.com)

#ifndef FAST_INNOCHECKSUM_UTIL_H_
#define FAST_INNOCHECKSUM_UTIL_H_

#include <stdlib.h>

#include "ut0crc32.h"

#define LOG_LEVEL_INFO 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_TRACE 2
#define LOG_LEVEL LOG_LEVEL_INFO

#define USE_DIRECT_IO 1

#define SKIP_ERROR 0

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);             \
    void operator=(const TypeName&)

enum RetCode
{
    kSucc = 0,
    kError = 1,
    kCorruption = 2,
    kNotSupported = 3,
    kInvalidArgument = 4,
    kIOError = 5,
    kIncomplete = 6,
    kTimedOut = 7,
    kFull = 8,
    kOutOfMemory = 9,
};

#define FIL_PAGE_LSN 16
#define FIL_PAGE_FILE_FLUSH_LSN 26
#define FIL_PAGE_OFFSET 4
#define FIL_PAGE_TYPE 24
#define FIL_PAGE_DATA 38
#define FIL_PAGE_END_LSN_OLD_CHKSUM 8
#define FIL_PAGE_SPACE_OR_CHKSUM 0
#define UNIV_PAGE_SIZE (2 * 8192)

/* innodb function in name; modified slightly to not have the ASM version (lots
 * of #ifs that didn't apply) */
inline ulint mach_read_from_4(uchar* b)
{
    return (((ulint)(b[0]) << 24) + ((ulint)(b[1]) << 16) +
            ((ulint)(b[2]) << 8) + (ulint)(b[3]));
}

ulint buf_calc_page_crc32(uchar* page);

#endif  // FAST_INNOCHECKSUM_UTIL_H_