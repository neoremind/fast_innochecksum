/* Copyright (C) 2000-2005 MySQL AB & Innobase Oy

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA */

/*
  InnoDB offline file checksum utility.  85% of the code in this file
  was taken wholesale fron the InnoDB codebase.

  The final 15% was originally written by Mark Smith of Danga
  Interactive, Inc. <junior@danga.com>

  Published with a permission.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "scanner.h"
#include "util.h"

uint64_t NowMicros()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

static const uint64_t kShardByteSize = 256 * 1024 * 1024;
static const uint32_t kNumOfScanCachePartition = 3;
static const uint64_t kPartitionByteSize = 16 * 1024 * 1024;

int main(int argc, char** argv)
{
    struct stat st;              /* for stat, if you couldn't guess */
    unsigned long long int size; /* size of file (has to be 64 bits) */
    ulint pages;                 /* number of pages in file */
    int justCount = 0;    /* if true, just print page count */
    uint32_t numOfConcurrentShard = 1;

    int c;

    /* remove arguments */
    while ((c = getopt(argc, argv, "cn:")) != -1)
    {
        switch (c)
        {
            case 'c':
                justCount = 1;
                break;
            case 'n':
                numOfConcurrentShard = atoi(optarg);
                break;
            case ':':
                fprintf(stderr, "option -%c requires an argument\n", optopt);
                return 1;
                break;
            case '?':
                fprintf(stderr, "unrecognized option: -%c\n", optopt);
                return 1;
                break;
        }
    }

    /* make sure we have the right arguments */
    if (optind >= argc)
    {
        printf("InnoDB offline file checksum utility.\n");
        printf(
            "usage: %s [-c] [-n <num of concurrent shard>] <filename>\n",
            argv[0]);
        printf("\t-c\tprint the count of pages in the file\n");
        printf("\t-n n\tnumber of concurrent shard to scan, default is 1\n");
        return 1;
    }

    /* stat the file to get size and page count */
    if (stat(argv[optind], &st))
    {
        perror("error statting file");
        return 1;
    }
    size = st.st_size;
    pages = size / UNIV_PAGE_SIZE;
    if (justCount)
    {
        printf("%lu\n", pages);
        return 0;
    }

    uint64_t start_time, end_time;
    start_time = NowMicros();

    ut_crc32_init();

    Scanner scanner(std::string(argv[optind]), size, kShardByteSize,
                    kNumOfScanCachePartition, kPartitionByteSize,
                    numOfConcurrentShard);
    scanner.Execute();

    end_time = NowMicros();
    printf("time elapsed microsecond(us) %lld, %lld MB/s\n",
           end_time - start_time, size / (end_time - start_time));

    return 0;
}