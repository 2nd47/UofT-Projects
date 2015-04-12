For the recorded results, I ran:

./sim -f ./traceprogs/<TRACE FILE> -m <MEMORY SIZE> -s 3000 -a <ALGORITHM>
DISCLAIMER: I DID NOT RUN IT AGAINST THE .../a2-verify/ TRACES FOR THE TABLE VALUES

As well, the personal file I ran was a trace of a garbage collection program which I wrote last semester for CSC209.

A comparison of the algorithms:
	Overall, the rand algorithm provided the lowest hit rate out of all algorithms. FIFO provides results at around the same range as rand and can either have worse or better results than RAND. LRU often provides the best result compared to any other algorithm except for OPT. CLOCK, in general, performed similar to LRU but provided worse results than LRU. OPT was, of course, optimal. As memory capacity increases, all algorithms perform better than before.

An analysis of the LRU algorithm with an increase in memory:
	For matmul in particular, the LRU algorithm was not very efficient until it had a memory size of 150 to work with. However, for simpleloop, the improvement was very slow and minimal. For blocked, the improvment was, again, slow and minimal. Though, it began very promisingly for blocked, much like other algorithms.
	
Furthermore, I can guarantee that my OPT algorith runs in O(m*n) (m pages of memory, n references in trace) time because:
	I index every refernce only once in my own data structure, a O(n) space complexity.
	Then, for each m page of memory, it is referenced at worst n times because there are n references which are done and each reference looks through, at worst, every page of memory. However, because we can index directly into my data structure, the cost of finding a reference in it is O(1). Then, all other calculations lead to O(m*n) because they do not loop through different references and, at worst (if there is only one reference or all references are only referenced once), looks through the entries n times (which is done m times for each page of loaded memory).