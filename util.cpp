// Use of this source code is governed by GNU General Public License
// that can be found in the License file.
//
// Author: neoremind (neoremind.com)

#include "util.h"

ulint buf_calc_page_crc32(uchar* page)
{
    /* Since the field FIL_PAGE_FILE_FLUSH_LSN, and in versions <= 4.1.x
    FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID, are written outside the buffer pool
    to the first pages of data files, we have to skip them in the page
    checksum calculation.
    We must also skip the field FIL_PAGE_SPACE_OR_CHKSUM where the
    checksum is stored, and also the last 8 bytes of page because
    there we store the old formula checksum. */

    const uint32_t c1 = ut_crc32(page + FIL_PAGE_OFFSET,
                                 FIL_PAGE_FILE_FLUSH_LSN - FIL_PAGE_OFFSET);

    const uint32_t c2 = ut_crc32(page + FIL_PAGE_DATA,
                 UNIV_PAGE_SIZE - FIL_PAGE_DATA - FIL_PAGE_END_LSN_OLD_CHKSUM);

    return (c1 ^ c2);
}