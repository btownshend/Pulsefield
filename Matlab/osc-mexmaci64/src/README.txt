Open Sound Control send and receive functions for Matlab.

Written by Andy Schmeder
License for use and redistribution: GNU Public License v2 or later.

Updates / source: http://andy.schmeder.net/software

Questions / comment / bugs?  mailto:andy.schmeder@gmail.com

Requires liblo 0.22 or later.

---

HOW TO INSTALL THE PRECOMPILED BINARIES:

1. Figure out what your operating platform is, and obtain the correct file.  

  - Windows: mexwin
  - Mac Intel 32-bit: mexmaci
  - Mac Intel 64-bit: mexmaci64
  - PPC Mac: mexmac

  Download the file and save it to your computer.  Extract the files and put them into a folder somewhere sensible.  For example, ~/Matlab/oscmex.

2. Make sure that the files are located in the Matlab search path.

  See the Matlab documentation for the function "addpath" for more information about the search path.
  Try to run the example, osc_client.m.

---

HOW TO RECOMPILE:

1. Download and compile Liblo.  You will want to make sure the compiler and flags are the same ones used by the Matlab compiler command "mex".

  For example, on Intel Mac 64-bit I use this to compile Liblo.

  cd /usr/local/src/liblo-0.26
  CC=/usr/bin/gcc-4.0 CFLAGS="-arch x86_64" ./configure
  make && make install


2. Run the "osc_make.m" script from Matlab.

  You may need to edit osc_make.m if the Liblo files got installed somewhere other than /usr/local.

3. Install the compiled files so they are in the Matlab search path.

---

Dec 2010

- 64-bit support.

Jan 2009

- Packaged for intel mac.  No more support for mac ppc.
- Fixed crash with recv'd messages containing strings longer than 32 characters

