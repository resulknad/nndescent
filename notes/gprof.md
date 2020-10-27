# gprof
The gprof perfomance analysis tool is used to keep track of the number of the similarity measurements. Using the compile flag -pg and then running the code writes some statistical informations about the flat profile and the call graph to "gmon.out".

## commands
```bash
gcc -pg -O3 -ffast-math -march=native -o a.out knnd.c knnd_test.c vec.c -lm
./a.out data out.o 4000 4 20
gprof ./a.out
```

### Links:
* https://sourceware.org/binutils/docs/gprof/Implementation.html
* https://bytes.com/topic/c/answers/754125-counting-function-calls
* https://ftp.gnu.org/old-gnu/Manuals/gprof-2.9.1/html_chapter/gprof_5.html


### sample output:
```bash
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total           
 time   seconds   seconds    calls  ms/call  ms/call  name    
 50.07      0.01     0.01   274783     0.00     0.00  heap_insert
 50.07      0.02     0.01        1    10.01    20.03  nn_descent
  0.00      0.02     0.00   656642     0.00     0.00  heap_find_by_index
  0.00      0.02     0.00   221431     0.00     0.00  l2
  0.00      0.02     0.00    17555     0.00     0.00  max_heapify
  0.00      0.02     0.00     7458     0.00     0.00  vec_clear
  0.00      0.02     0.00     5120     0.00     0.00  heap_union
  0.00      0.02     0.00     1281     0.00     0.00  vec_create
  0.00      0.02     0.00     1281     0.00     0.00  vec_free

 %         the percentage of the total running time of the
time       program used by this function.

cumulative a running sum of the number of seconds accounted
 seconds   for by this function and those listed above it.

 self      the number of seconds accounted for by this
seconds    function alone.  This is the major sort for this
           listing.

calls      the number of times this function was invoked, if
           this function is profiled, else blank.

 self      the average number of milliseconds spent in this
ms/call    function per call, if this function is profiled,
	   else blank.

 total     the average number of milliseconds spent in this
ms/call    function and its descendents per call, if this
	   function is profiled, else blank.

name       the name of the function.  This is the minor sort
           for this listing. The index shows the location of
	   the function in the gprof listing. If the index is
	   in parenthesis it shows where it would appear in
	   the gprof listing if it were to be printed.


Copyright (C) 2012-2020 Free Software Foundation, Inc.

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.
```

### Counting compares
`gcc knnd.c knnd_test.c vec.c  -lm -O3 -ffast-math -march=native -pg -ggdb -fno-inline-small-functions -D"INSTR=true"`

```
#ifdef INSTR
 int log_fp_comp();
 
 #define FP_COMP() log_fp_comp()
 #define INSTR_ONCE 1
 #else
 #define FP_COMP() true;
 #endif
 
 
 #pragma GCC push_options
 #pragma GCC optimize ("O0")
 
 int log_fp_comp() {
     return 1;
 }
 #pragma GCC pop_options
```

 
 Then insert macro FP_COMP() on every comparison. For high-dimensional data compares don't matter (<1%), for lower dimensional data its something like 8%.