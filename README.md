# Fast innochecksum

Fast-innochecksum aims accelerate official `innochecksum` bin by leveraging

* IO and computation separation.
* Big block IO to amortize too much kernel IO overhead.
* Make full use of multi-core to work parallelly.

For details of the implementation, please refer to [优化innochecksum实战，四倍加速榨干NVMe SSD磁盘带宽](https://zhuanlan.zhihu.com/p/178670421) (in Chinese).

## Build

To make debug bin:

```
make dbg
```

To make production bin:

```
make
```

## Run

By default, single thread is used.

```
./fast_innochecksum <file_name>.ibd
```

To enable multiple threads scanning.

```
./fast_innochecksum -n 16 <file_name>.ibd
```

## Configuration

util.h

```
#define LOG_LEVEL_INFO 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_TRACE 2
#define LOG_LEVEL LOG_LEVEL_INFO

#define USE_DIRECT_IO 1

#define SKIP_ERROR 0
```

fast_innochecksum.cpp

```
static const uint64_t kShardByteSize = 256 * 1024 * 1024;
static const uint32_t kNumOfScanCachePartition = 3;
static const uint64_t kPartitionByteSize = 16 * 1024 * 1024;
```

