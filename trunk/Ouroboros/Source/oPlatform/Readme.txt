= oPlatform =

== oTask Implementation Details ==

oBasis/oTask.h and oBasis/oDispatchQueueConcurrent.h and oBasis/oStdFuture.h declare functions that are not implemented in oBasis. At this time the Ouroboros library defines operating system aware task/thread pool systems as platform-specific. (i.e. would you use TBB on an Apple platform? or Grand Central?) It would be a shame to loose the speed of parallelism in otherwise generic algorithms, which is why this dangling declaration was done. In oPlatform the implementation must be defined, and as is often the case only one scheduler should be choosen for the system. TBB requires a DLL external to an EXE, GC and PPL are truly platform-specific and other implementations are not nearly as efficient. To allow flexibility the code to implement Ouroboros requirements is isolated to its own small library. For each executable, linking to a library will be sufficient to fulfilly the API reqirements of oTask/oDispatchQueueConcurrent/oStdFuture.

=== Current Options ===

# oPlatformTBB: uses Intel's Thread Building Blocks. Use this, it is truly awesome.
# oPlatformPPL: Microsoft's recent Parallel Patterns Library and Concurrency (concrt) implementation. Simply not as performant as TBB yet.
# oPlatformSerial: If you suspect a race condition, put this in place to see. SLOW!
# oPlatformGeneric: Uses custom implementations as a reference for benchmarking or for early bringup of Ouroboros on a new platform.

=== Important Notes ===

* All DLLs used by an executable should use the same scheduler. This is not always possible, such as with MS's Kinect driver DLLs if the application uses TBB, but that's the price of middleware.

* The main motivation beyond cross-platform support is that small utility EXEs can be created without the added complexity of an external TBB DLL.

* Use TBB whereever performance is important. It's simply faster and more stable (less performance spikes) by a significant margin from anything we've benchmarked.

