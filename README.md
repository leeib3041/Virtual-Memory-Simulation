# Virtual-Memory-Simulation
Program to simulate a paged virtual memory system. It will read a virtual addresses and maintain counts of memory accesses, TLB misses, and page faults.

### The simulation system uses:
* 24 bit virtual addresses  - 16-bit virtual page number (VPN) & 8-bit page offset
* 16 bit physical addresses  - 8-bit physical frame number (PFN) & 8-bit page offset
 
The simulated system also contains a single-level TLB that provides the VPN-to-FPN translation when there is a TLB hit.
 
This is a simple simulation, the memory accesses are not categorized as reads, instruction fetches, or writes. Although, it can be added to the implementation in the future. Therefore, all the accesses are treated as reads, so there is no need to track dirty pages and no need to verify page access permissions.
``` 
 Page table is indexed by VPN - pseudo-LRU policy based on vector of use bits
   Core map is indexed by PFN - pseudo-LRU policy based on vector of use bits
                          TLB - fully-associative (can be simulated with  a linear search, FIFO policy
```
Physical memory is not simulated in this program

### Files include:
* paging.cpp   - program
* Makefile     - simple makefile, nothing special
* trace1       - basic test, uses paging.cfg
* trace2       - all misses (TLB and page faults) test, uses paging.cfg
* trace3       - forces verbose print out in middle of execution test if hex digts are > 6, uses paging.cfg
* trace4       - a more complicated testing, use paging1.cfg (must be changed within the code)
* paging.cfg   
* paging1.cfg  
 
### paging config files have 3 lines each:
* PF (# of page frame)                                         
* TE (# of table entries)                                      
* UP (# of memory accesses for an observation period of usage, this is the access count after which the current use bit              vector is shifted right)
 
### Makefile will provide the compilation, but there is no make run since test runs change based on file input:
```
                  $ make
 main execution:  $ ./paging < trace1, 
                  $ ./paging < trace2, 
                  $ ./paging < trace3, 
                  $ ./paging < trace4
 another method:  $ head -10 trace# | ./paging
 another method:  $ ./paging -v < trace#    
                  $ make clean
```
"-v" parameter is a verbose output, when used it will printout the details of each address transaction. Without it, it will just print the amount of access, TLB misses, and Page faults*

Trace files contain 24-bit hex values (6 hex digits) representing virtual memory

### ADDRESS TRANSLATION DECISION TREE

This is a high-level design of decisions and actions. You must
further include the appropriate updates to the maintained counts.
```
1. Check the TLB

  2A. If there is a TLB hit, use the PFN from the TLB entry to obtain the physical address. You are done.
  
  2B. If there is a TLB miss, check the page table.

      3B. If the presence bit is on, this is a page hit. Use the PFN from the page table entry to obtain the physicaL address.           Update the TLB. You are done.
    
      3B. If the presence bit is off, this is a page fault. CheCK for free frames.

          4A. If there is a free frame, map the virtual page to that frame by updating the page table and the core map. Update               the TLB. You are done.
       
          4B. If there is no free frame, search the core map for the first frame with the lowest-valued use vector. Replace                 that frame. Update the page table and core map to reflect that the old mapping is broken. Invalidate the TLB                   entry for the old mapping, if one exists in the TLB. Update the page table and core map to reflect the new                     mapping. Update the TLB. You are done.
```

### PAGE REPLACEMENT

When handling a page fault, you should look for an empty frame
in the core map (i.e., valid == 0) into which you can place the
page. If all frames are valid, examine the use vectors in the
core map to determine a frame to replace.

On each use of a page frame, record a 1 in the high bit of the
use vector. One of the parameters to the simulation is the length
of an observation period of usage. The parameter is an access
count. After this access count occurs, shift all the use vectors
in the core map. This has the effect of recently-used page having
the high-order bits set and not-recently-use pages having no bits
set or only low-order bits set. If you treat the use vectors as
unsign integers, the one with the lowest integer value will likely
be the least recently used page. With a limited number of bits,
this will not be true LRU. However, it can be a close approximation
based on the number of bits in the bit vector and the length of
the observation period. (More bits and shorter observation periods
are better.)

(The optimization of setting the high-order use bit only on a TLB
miss is not appropriate unless you combine it with flushing the
TLB at the end of each observation period.)


### TLB REPLACEMENT

When handling a TLB misss, you should look for an empty entry
(i.e., valid == 0) into which you can place a new entry. If all
entries are valid, use a FIFO replacement index to choose a valid
entry to replace. The index should start at 0 and be incremented
each time you use it to make a replacement index. You should modulo
the incremented value by the number of TLB entries so that the
index will stay in range.

### Test outputs:
```
These examples use the configuration values as given above:

PF 4
TE 2
UP 4


Trace file 1:

000000
000101
000202
000303
000000
000101
000202
000303
000000
000101
000202
000303


**Ouput of ./a.out < trace1

statistics
  accesses    = 12
  tlb misses  = 12
  page faults = 4


**Output of ./a.out -v < trace1

paging simulation
  65536 virtual pages in the virtual address space
  4 physical page frames
  2 TLB entries
  use vectors in core map are shifted every 4 accesses

access 1:
  virtual address is              0x000000
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0000
  tlb update of vpn 0x0000 with pfn 0x00
access 2:
  virtual address is              0x000101
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0101
  tlb update of vpn 0x0001 with pfn 0x01
access 3:
  virtual address is              0x000202
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0202
  tlb update of vpn 0x0002 with pfn 0x02
access 4:
  virtual address is              0x000303
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0303
  tlb update of vpn 0x0003 with pfn 0x03
shift use vectors
access 5:
  virtual address is              0x000000
  tlb miss
  page hit, physical address is     0x0000
  tlb update of vpn 0x0000 with pfn 0x00
access 6:
  virtual address is              0x000101
  tlb miss
  page hit, physical address is     0x0101
  tlb update of vpn 0x0001 with pfn 0x01
access 7:
  virtual address is              0x000202
  tlb miss
  page hit, physical address is     0x0202
  tlb update of vpn 0x0002 with pfn 0x02
access 8:
  virtual address is              0x000303
  tlb miss
  page hit, physical address is     0x0303
  tlb update of vpn 0x0003 with pfn 0x03
shift use vectors
access 9:
  virtual address is              0x000000
  tlb miss
  page hit, physical address is     0x0000
  tlb update of vpn 0x0000 with pfn 0x00
access 10:
  virtual address is              0x000101
  tlb miss
  page hit, physical address is     0x0101
  tlb update of vpn 0x0001 with pfn 0x01
access 11:
  virtual address is              0x000202
  tlb miss
  page hit, physical address is     0x0202
  tlb update of vpn 0x0002 with pfn 0x02
access 12:
  virtual address is              0x000303
  tlb miss
  page hit, physical address is     0x0303
  tlb update of vpn 0x0003 with pfn 0x03
shift use vectors

statistics
  accesses    = 12
  tlb misses  = 12
  page faults = 4

tlb
  valid = 1, vpn = 0x0002, pfn = 0x02
  valid = 1, vpn = 0x0003, pfn = 0x03

core map table
  pfn = 0x00: valid = 1, use vector = 0x70, vpn = 0x0000
  pfn = 0x01: valid = 1, use vector = 0x70, vpn = 0x0001
  pfn = 0x02: valid = 1, use vector = 0x70, vpn = 0x0002
  pfn = 0x03: valid = 1, use vector = 0x70, vpn = 0x0003

first ten entries of page table
  vpn = 0x0000: presence = 1, pfn = 0x00
  vpn = 0x0001: presence = 1, pfn = 0x01
  vpn = 0x0002: presence = 1, pfn = 0x02
  vpn = 0x0003: presence = 1, pfn = 0x03
  vpn = 0x0004: presence = 0, pfn = 0x00
  vpn = 0x0005: presence = 0, pfn = 0x00
  vpn = 0x0006: presence = 0, pfn = 0x00
  vpn = 0x0007: presence = 0, pfn = 0x00
  vpn = 0x0008: presence = 0, pfn = 0x00
  vpn = 0x0009: presence = 0, pfn = 0x00
 
************************************************************************************
Trace file 2:

000000
000101
000202
000303
000404
000505
000606
000000
000101
000202
000303
000404

**Output of ./a.out < trace2

statistics
  accesses    = 12
  tlb misses  = 12
  page faults = 12


**Output of ./a.out -v < trace2

paging simulation
  65536 virtual pages in the virtual address space
  4 physical page frames
  2 TLB entries
  use vectors in core map are shifted every 4 accesses

access 1:
  virtual address is              0x000000
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0000
  tlb update of vpn 0x0000 with pfn 0x00
access 2:
  virtual address is              0x000101
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0101
  tlb update of vpn 0x0001 with pfn 0x01
access 3:
  virtual address is              0x000202
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0202
  tlb update of vpn 0x0002 with pfn 0x02
access 4:
  virtual address is              0x000303
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0303
  tlb update of vpn 0x0003 with pfn 0x03
shift use vectors
access 5:
  virtual address is              0x000404
  tlb miss
  page fault
  page replacement needed
  TLB invalidate of vpn 0x0
  replace frame 0
  physical address is               0x0004
  tlb update of vpn 0x0004 with pfn 0x00
access 6:
  virtual address is              0x000505
  tlb miss
  page fault
  page replacement needed
  TLB invalidate of vpn 0x1
  replace frame 1
  physical address is               0x0105
  tlb update of vpn 0x0005 with pfn 0x01
access 7:
  virtual address is              0x000606
  tlb miss
  page fault
  page replacement needed
  TLB invalidate of vpn 0x2
  replace frame 2
  physical address is               0x0206
  tlb update of vpn 0x0006 with pfn 0x02
access 8:
  virtual address is              0x000000
  tlb miss
  page fault
  page replacement needed
  TLB invalidate of vpn 0x3
  replace frame 3
  physical address is               0x0300
  tlb update of vpn 0x0000 with pfn 0x03
shift use vectors
access 9:
  virtual address is              0x000101
  tlb miss
  page fault
  page replacement needed
  TLB invalidate of vpn 0x4
  replace frame 0
  physical address is               0x0001
  tlb update of vpn 0x0001 with pfn 0x00
access 10:
  virtual address is              0x000202
  tlb miss
  page fault
  page replacement needed
  TLB invalidate of vpn 0x5
  replace frame 1
  physical address is               0x0102
  tlb update of vpn 0x0002 with pfn 0x01
access 11:
  virtual address is              0x000303
  tlb miss
  page fault
  page replacement needed
  TLB invalidate of vpn 0x6
  replace frame 2
  physical address is               0x0203
  tlb update of vpn 0x0003 with pfn 0x02
access 12:
  virtual address is              0x000404
  tlb miss
  page fault
  page replacement needed
  TLB invalidate of vpn 0x0
  replace frame 3
  physical address is               0x0304
  tlb update of vpn 0x0004 with pfn 0x03
shift use vectors

statistics
  accesses    = 12
  tlb misses  = 12
  page faults = 12

tlb
  valid = 1, vpn = 0x0003, pfn = 0x02
  valid = 1, vpn = 0x0004, pfn = 0x03

core map table
  pfn = 0x00: valid = 1, use vector = 0x40, vpn = 0x0001
  pfn = 0x01: valid = 1, use vector = 0x40, vpn = 0x0002
  pfn = 0x02: valid = 1, use vector = 0x40, vpn = 0x0003
  pfn = 0x03: valid = 1, use vector = 0x40, vpn = 0x0004

first ten entries of page table
  vpn = 0x0000: presence = 0, pfn = 0x00
  vpn = 0x0001: presence = 1, pfn = 0x00
  vpn = 0x0002: presence = 1, pfn = 0x01
  vpn = 0x0003: presence = 1, pfn = 0x02
  vpn = 0x0004: presence = 1, pfn = 0x03
  vpn = 0x0005: presence = 0, pfn = 0x00
  vpn = 0x0006: presence = 0, pfn = 0x00
  vpn = 0x0007: presence = 0, pfn = 0x00
  vpn = 0x0008: presence = 0, pfn = 0x00
  vpn = 0x0009: presence = 0, pfn = 0x00

************************************************************************************

Trace file 3: (The hex value 1000000 triggers a display midway
  through the trace.)
000000
000101
000202
1000000
000000
000202

**Output of ./a.out < trace3

statistics
  accesses    = 3
  tlb misses  = 3
  page faults = 3

tlb
  valid = 1, vpn = 0x0002, pfn = 0x02
  valid = 1, vpn = 0x0001, pfn = 0x01

core map table
  pfn = 0x00: valid = 1, use vector = 0x80, vpn = 0x0000
  pfn = 0x01: valid = 1, use vector = 0x80, vpn = 0x0001
  pfn = 0x02: valid = 1, use vector = 0x80, vpn = 0x0002
  pfn = 0x03: valid = 0, use vector = 0x00, vpn = 0x0000

first ten entries of page table
  vpn = 0x0000: presence = 1, pfn = 0x00
  vpn = 0x0001: presence = 1, pfn = 0x01
  vpn = 0x0002: presence = 1, pfn = 0x02
  vpn = 0x0003: presence = 0, pfn = 0x00
  vpn = 0x0004: presence = 0, pfn = 0x00
  vpn = 0x0005: presence = 0, pfn = 0x00
  vpn = 0x0006: presence = 0, pfn = 0x00
  vpn = 0x0007: presence = 0, pfn = 0x00
  vpn = 0x0008: presence = 0, pfn = 0x00
  vpn = 0x0009: presence = 0, pfn = 0x00

statistics
  accesses    = 5
  tlb misses  = 4
  page faults = 3


**Output of ./a.out < trace3

paging simulation
  65536 virtual pages in the virtual address space
  4 physical page frames
  2 TLB entries
  use vectors in core map are shifted every 4 accesses

access 1:
  virtual address is              0x000000
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0000
  tlb update of vpn 0x0000 with pfn 0x00
access 2:
  virtual address is              0x000101
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0101
  tlb update of vpn 0x0001 with pfn 0x01
access 3:
  virtual address is              0x000202
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0202
  tlb update of vpn 0x0002 with pfn 0x02

statistics
  accesses    = 3
  tlb misses  = 3
  page faults = 3

tlb
  valid = 1, vpn = 0x0002, pfn = 0x02
  valid = 1, vpn = 0x0001, pfn = 0x01

core map table
  pfn = 0x00: valid = 1, use vector = 0x80, vpn = 0x0000
  pfn = 0x01: valid = 1, use vector = 0x80, vpn = 0x0001
  pfn = 0x02: valid = 1, use vector = 0x80, vpn = 0x0002
  pfn = 0x03: valid = 0, use vector = 0x00, vpn = 0x0000

first ten entries of page table
  vpn = 0x0000: presence = 1, pfn = 0x00
  vpn = 0x0001: presence = 1, pfn = 0x01
  vpn = 0x0002: presence = 1, pfn = 0x02
  vpn = 0x0003: presence = 0, pfn = 0x00
  vpn = 0x0004: presence = 0, pfn = 0x00
  vpn = 0x0005: presence = 0, pfn = 0x00
  vpn = 0x0006: presence = 0, pfn = 0x00
  vpn = 0x0007: presence = 0, pfn = 0x00
  vpn = 0x0008: presence = 0, pfn = 0x00
  vpn = 0x0009: presence = 0, pfn = 0x00

access 4:
  virtual address is              0x000000
  tlb miss
  page hit, physical address is     0x0000
  tlb update of vpn 0x0000 with pfn 0x00
shift use vectors
access 5:
  virtual address is              0x000202
  tlb hit, physical address is      0x0202

statistics
  accesses    = 5
  tlb misses  = 4
  page faults = 3

tlb
  valid = 1, vpn = 0x0002, pfn = 0x02
  valid = 1, vpn = 0x0000, pfn = 0x00

core map table
  pfn = 0x00: valid = 1, use vector = 0x40, vpn = 0x0000
  pfn = 0x01: valid = 1, use vector = 0x40, vpn = 0x0001
  pfn = 0x02: valid = 1, use vector = 0xc0, vpn = 0x0002
  pfn = 0x03: valid = 0, use vector = 0x00, vpn = 0x0000

first ten entries of page table
  vpn = 0x0000: presence = 1, pfn = 0x00
  vpn = 0x0001: presence = 1, pfn = 0x01
  vpn = 0x0002: presence = 1, pfn = 0x02
  vpn = 0x0003: presence = 0, pfn = 0x00
  vpn = 0x0004: presence = 0, pfn = 0x00
  vpn = 0x0005: presence = 0, pfn = 0x00
  vpn = 0x0006: presence = 0, pfn = 0x00
  vpn = 0x0007: presence = 0, pfn = 0x00
  vpn = 0x0008: presence = 0, pfn = 0x00
  vpn = 0x0009: presence = 0, pfn = 0x00
  
************************************************************************************
paging.cfg

PF 5
TE 3
UP 1

Trace file 4

012345
6789ab
cdef01
234567
89abcd
ef0123
456789
abcdef

**Output of ./paging -v < trace4

paging simulation
  65536 virtual pages in the virtual address space
  5 physical page frames
  3 TLB entries
  use vectors in core map are shifted every 1 accesses

access 1:
  virtual address is              0x012345
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0045
  tlb update of vpn 0x0123 with pfn 0x00
shift use vectors
access 2:
  virtual address is              0x6789ab
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x01ab
  tlb update of vpn 0x6789 with pfn 0x01
shift use vectors
access 3:
  virtual address is              0xcdef01
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0201
  tlb update of vpn 0xcdef with pfn 0x02
shift use vectors
access 4:
  virtual address is              0x234567
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x0367
  tlb update of vpn 0x2345 with pfn 0x03
shift use vectors
access 5:
  virtual address is              0x89abcd
  tlb miss
  page fault
  unused page frame allocated
  physical address is               0x04cd
  tlb update of vpn 0x89ab with pfn 0x04
shift use vectors
access 6:
  virtual address is              0xef0123
  tlb miss
  page fault
  page replacement needed
  TLB invalidate of vpn 0x123
  replace frame 0
  physical address is               0x0023
  tlb update of vpn 0xef01 with pfn 0x00
shift use vectors
access 7:
  virtual address is              0x456789
  tlb miss
  page fault
  page replacement needed
  TLB invalidate of vpn 0x6789
  replace frame 1
  physical address is               0x0189
  tlb update of vpn 0x4567 with pfn 0x01
shift use vectors
access 8:
  virtual address is              0xabcdef
  tlb miss
  page fault
  page replacement needed
  TLB invalidate of vpn 0xcdef
  replace frame 2
  physical address is               0x02ef
  tlb update of vpn 0xabcd with pfn 0x02
shift use vectors

statistics
  accesses    = 8
  tlb misses  = 8
  page faults = 8

tlb
  valid = 1, vpn = 0x4567, pfn = 0x01
  valid = 1, vpn = 0xabcd, pfn = 0x02
  valid = 1, vpn = 0xef01, pfn = 0x00

core map table
  pfn = 0x00: valid = 1, use vector = 0x10, vpn = 0xef01
  pfn = 0x01: valid = 1, use vector = 0x20, vpn = 0x4567
  pfn = 0x02: valid = 1, use vector = 0x40, vpn = 0xabcd
  pfn = 0x03: valid = 1, use vector = 0x04, vpn = 0x2345
  pfn = 0x04: valid = 1, use vector = 0x08, vpn = 0x89ab

first ten entries of page table
  vpn = 0x0000: presence = 0, pfn = 0x00
  vpn = 0x0001: presence = 0, pfn = 0x00
  vpn = 0x0002: presence = 0, pfn = 0x00
  vpn = 0x0003: presence = 0, pfn = 0x00
  vpn = 0x0004: presence = 0, pfn = 0x00
  vpn = 0x0005: presence = 0, pfn = 0x00
  vpn = 0x0006: presence = 0, pfn = 0x00
  vpn = 0x0007: presence = 0, pfn = 0x00
  vpn = 0x0008: presence = 0, pfn = 0x00
  vpn = 0x0009: presence = 0, pfn = 0x00
```
