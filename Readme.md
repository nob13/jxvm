JxVm
====
* A mini incomplete VM for java byte code.
* Note: far from complete, maybe 90% of the simpler op codes are implemented.
* However can run a Hello World Application.
* Performance is slow, there is no linker yet and all lookups are done via Strings.
* No Exception (only throwing, but not catching)/Garbage Collection/Threading yet
* Needs a real java rutime library (e.g. OpenJDK) to start.
* Java methods can be overriden via C++-Methods
* Whole Java classes can be replaced (`javalib`). This is needed for stubbing out Thread-related startup code

Why?
====
* I Wanted to know internals of Java apps better.
* It made some fun.
   * Hello World takes 90000 Operations to run through.


Documentation used
------------------

* Nicer description of JVM Spec: http://cs.au.dk/~mis/dOvs/jvmspec/ref--35.html
* Class file format: https://docs.oracle.com/javase/specs/jvms/se8/html/jvms-4.html#jvms-4.10.1.9.lconst_l
* Opcode list: https://docs.oracle.com/javase/specs/jvms/se8/html/jvms-7.html

How to build/Run
----------------

Requirements

 * Boost
 * Only tested OSX 10.11 so far.
 
        # Java standard library is found via JAVA_HOME
        export JAVA_HOME=/* path to some java VM where it can find jre/lib/rt.jar */
        ./build_and_test.sh

        # Running Hello World.
        cd build/apps
        ./jxvm ../../manual_test/hello_world/HelloWorld.class
     

License
-------
* MIT

