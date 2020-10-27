# perf informations
Perf can be used for profiling by directly accessing CPU hardware registers to obtain performance counters for a wide range of events.
### follow this guide:
http://www.bnikolic.co.uk/blog/hpc-prof-events.html
http://www.bnikolic.co.uk/blog/hpc-howto-measure-flops.html

* extract the events you want to track (using showevtinfo)
* extract code of the event (using check_events)
*
perf events:


#### only pre-defined events
```bash
perf stat -e cycles,cache-references,cache-misses,L1-dcache-load-misses,L1-dcache-loads,L1-dcache-stores,L1-icache-load-misses,LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,page-faults ./a.out data out.o 4000 4 20
```

#### only code events (machine dependent)
```bash
perf stat -e r0 -e r10000 -e r2 -e r10002 -e r5301c7 -e r5302c7 -e r5304c7 -e r5308c7 -e r5310c7 -e r5320c7 -e r5340c7 -e r5380c7 ./a.out data out.o 4000 4 20
```

#### all events
```bash
perf stat -e cycles,cache-references,cache-misses,L1-dcache-load-misses,L1-dcache-loads,L1-dcache-stores,L1-icache-load-misses,LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses,page-faults -e r0,r10000,r2,r10002,r5301c7,r5302c7,r5304c7,r5308c7,r5310c7,r5320c7,r5340c7,r5380c7 ./a.out data out.o 4000 4 20
```


### sample showevtinfo output:
```bash
-----------------------------
IDX	 : 106954793
PMU name : perf (perf_events generic PMU)
Name     : PERF_COUNT_HW_CACHE_L1D
Equiv	 : None
Flags    : None
Desc     : L1 data cache
Code     : 0x0
Umask-00 : 0x00 : PMU : [READ] : None : read access  --> r10000
Umask-01 : 0x100 : PMU : [WRITE] : None : write access
Umask-02 : 0x200 : PMU : [PREFETCH] : None : prefetch access
Umask-03 : 0x00 : PMU : [ACCESS] : None : hit access --> r0
Umask-04 : 0x10000 : PMU : [MISS] : None : miss access --> r10000 (READ:MISS)



IDX	 : 106954805
PMU name : perf (perf_events generic PMU)
Name     : PERF_COUNT_HW_CACHE_LL
Equiv	 : None
Flags    : None
Desc     : Last level cache
Code     : 0x2
Umask-00 : 0x00 : PMU : [READ] : None : read access
Umask-01 : 0x100 : PMU : [WRITE] : None : write access
Umask-02 : 0x200 : PMU : [PREFETCH] : None : prefetch access
Umask-03 : 0x00 : PMU : [ACCESS] : None : hit access --> r2
Umask-04 : 0x10000 : PMU : [MISS] : None : miss access --> r10002


#-----------------------------
IDX	 : 419430469
PMU name : skl (Intel Skylake)
Name     : FP_ARITH
Equiv	 : FP_ARITH_INST_RETIRED
Flags    : None
Desc     : Floating-point instructions retired
Code     : 0xc7
Umask-00 : 0x01 : PMU : [SCALAR_DOUBLE] : None : Number of scalar double precision floating-point arithmetic instructions (multiply by 1 to get flops) --> r5301c7
Umask-01 : 0x02 : PMU : [SCALAR_SINGLE] : None : Number of scalar single precision floating-point arithmetic instructions (multiply by 1 to get flops) --> r5302c7
Umask-02 : 0x04 : PMU : [128B_PACKED_DOUBLE] : None : Number of scalar 128-bit packed double precision floating-point arithmetic instructions (multiply by 2 to get flops) -> r5304c7
Umask-03 : 0x08 : PMU : [128B_PACKED_SINGLE] : None : Number of scalar 128-bit packed single precision floating-point arithmetic instructions (multiply by 4 to get flops) -> r5308c7
Umask-04 : 0x10 : PMU : [256B_PACKED_DOUBLE] : None : Number of scalar 256-bit packed double precision floating-point arithmetic instructions (multiply by 4 to get flops) -> r5310c7
Umask-05 : 0x20 : PMU : [256B_PACKED_SINGLE] : None : Number of scalar 256-bit packed single precision floating-point arithmetic instructions (multiply by 8 to get flops) -> r5320c7
Umask-06 : 0x40 : PMU : [512B_PACKED_DOUBLE] : None : Number of scalar 512-bit packed double precision floating-point arithmetic instructions (multiply by 8 to get flops) -> r5340c7
Umask-07 : 0x80 : PMU : [512B_PACKED_SINGLE] : None : Number of scalar 512-bit packed single precision floating-point arithmetic instructions (multiply by 16 to get flops) -> r5380c7
Modif-00 : 0x00 : PMU : [k] : monitor at priv level 0 (boolean)
Modif-01 : 0x01 : PMU : [u] : monitor at priv level 1, 2, 3 (boolean)
Modif-02 : 0x02 : PMU : [e] : edge level (may require counter-mask >= 1) (boolean)
Modif-03 : 0x03 : PMU : [i] : invert (boolean)
Modif-04 : 0x04 : PMU : [c] : counter-mask in range [0-255] (integer)
Modif-05 : 0x05 : PMU : [t] : measure any thread (boolean)
Modif-06 : 0x07 : PMU : [intx] : monitor only inside transactional memory region (boolean)
Modif-07 : 0x08 : PMU : [intxcp] : do not count occurrences inside aborted transactional memory region (boolean)
```

#### sample perf output (codes noted at each event):
```bash
Performance counter stats for './a.out data out.o 4000 4 20':

    4,197,341,044      cycles:u                                                      (21.06%)
      136,544,543      cache-references:u                                            (21.30%)
       36,789,608      cache-misses:u            #   26.943 % of all cache refs      (21.79%)
       60,577,870      L1-dcache-load-misses:u   #    3.31% of all L1-dcache hits    (22.04%)
    1,832,294,068      L1-dcache-loads:u                                             (22.29%)
      207,055,845      L1-dcache-stores:u                                            (17.91%)
          392,086      L1-icache-load-misses:u                                       (17.67%)
       31,086,775      LLC-loads:u                                                   (17.67%)
        4,233,885      LLC-load-misses:u         #   13.62% of all LL-cache hits     (17.67%)
          338,868      LLC-stores:u                                                  (8.83%)
           86,800      LLC-store-misses:u                                            (8.83%)
            6,538      page-faults:u                                               
                0      r0:u                                                          (13.24%)
                0      r10000:u                                                      (17.66%)
                0      r2:u                                                          (17.66%)
                0      r10002:u                                                      (17.60%)
                5      r5301c7:u                                                     (17.36%)
       93,169,346      r5302c7:u                                                     (17.11%)
                0      r5304c7:u                                                     (17.11%)
                0      r5308c7:u                                                     (16.93%)
                0      r5310c7:u                                                     (16.93%)
                0      r5320c7:u                                                     (16.93%)
                0      r5340c7:u                                                     (16.69%)
                0      r5380c7:u                                                     (16.69%)

      1.222969366 seconds time elapsed

      1.184971000 seconds user
      0.029695000 seconds sys
  ```
