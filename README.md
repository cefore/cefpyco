# Cefore Python Compact package: cefpyco

- [Cefore Python Compact package: cefpyco](#cefore-python-compact-package-cefpyco)
  - [Overview](#overview)
  - [Installation](#installation)
    - [Installing Cefore](#installing-cefore)
    - [Installing Cefpyco (for Linux)](#installing-cefpyco-for-linux)
    - [Installing Cefpyco (for OS X)](#installing-cefpyco-for-os-x)
    - [Build.bash script](#buildbash-script)
    - [Uninstalling Cefpyco](#uninstalling-cefpyco)
  - [Configuration](#configuration)
    - [src/cefpyco](#srccefpyco)
    - [src/cefpyco\_wrap](#srccefpyco_wrap)
    - [src/cefpyco\_c](#srccefpyco_c)
    - [cefapp](#cefapp)
    - [test](#test)
  - [How to use cefpyco](#how-to-use-cefpyco)
    - [Connecting to and disconnecting from cefnetd](#connecting-to-and-disconnecting-from-cefnetd)
    - [Sending Interest packets](#sending-interest-packets)
    - [Sending Data packets](#sending-data-packets)
    - [Receiving packets](#receiving-packets)
    - [Creating Publisher App](#creating-publisher-app)
    - [Creating Consumer app](#creating-consumer-app)
  - [Notes](#notes)

## Overview

`cefpyco` is a Python package for Cefore applications.
It has the following features.

* Cefore functions, which are written in C, can be called from Python programs.
  * It is easier to develop Cefore applications in Python than in C.
* Interest and Data transmission/reception can be executed with a single function.
* Support for "with" blocks, enabling the easy programming of initialization and termination of connections with cefnetd.
* A simple application that sends back Data in response to receiving an Interest can be easily developed.
  * This means content can be provided without accessing csmgrd.
* Optional TLVs are supported (e.g., chunk number, termination chunk number, maximum hop count, Interest lifetime, content expiration, and cache expiration).
* InterestReturn packets are supported.
  * InterestReturn is an Interest packet returned only when an Interest packet is lost (it works like NACK).
  * Note that InterestReturn with "No Route" is returned when the upstream router or FIB is not appropriately set.

## Installation

### Installing Cefore	

First, install Cefore on your PC (target version: 0.10.0d or later).
To install Cefore, visit the Cefore website (https://cefore.net) or GitHub (https://github.com/cefore/cefore).
(The user manual is available at https://cefore.net/instruction and the source code is available at https://cefore.net/download).

### Installing Cefpyco (for Linux)

Execute the following commands in an environment where Cefore and Python are installed.
(Do not forget to add "." with cmake).

```bash
sudo apt-get install cmake python3-pip python3-dev python3-venv
pip3 install --upgrade build
pip3 install numpy click rich pytest pytest-sugar
cmake .
make
make install
```

After executing the above commands, you can `import cefpyco` from any directory.

For Python packages, setuptools is required for installation, but numpy, click, rich and pytest are options when using cefapp.

### Installing Cefpyco (for OS X)

This manual assumes an environment built using brew and pyenv.
Specifically, assume an environment installed with the following commands:

```bash
brew install cmake pyenv openssl
env PYTHON_CONFIGURE_OPTS='--enable-framework' pyenv install 3.x.x # Choose the version you want.
pyenv global 3.x.x
echo 'eval "$(pyenv init -)"' >> ~/.bash_profile # Option.
```

Note that you must set the environment variables when you run `pyenv install`; 
otherwise later `make` command will fail.

After rebooting your terminal or running `source ~/.bash_profile`, install the following packages:

```bash
python -m pip --upgrade pip build
python -m pip install numpy click rich pytest pytest-sugar
```

Because cmake cannot automatically find the Python and OpenSSL root directories, 
you need to set the necessary variables.
The root directory of the Python interpreter that you are using 
can be found with the following Python script:

```py
from sysconfig import get_config_var
print(get_config_var("base"))
```

The OpenSSL root directory can be found with `brew --prefix openssl`.

Using these paths, run the build process as follows:

```bash
py_root=... # Set by the above command.
ssl_root=... # Set by the above command.
cmake -DPython_ROOT_DIR="$py_root" -DOPENSSL_ROOT_DIR="$ssl_root" .
make
make install
```

### Build.bash script

We provide `build.bash` to support the build process 
except for installing the required libraries. 
Note that this script is experimental. 
If you run this script and encounter any problems, 
please return to the manual build procedure explained above.

### Uninstalling Cefpyco

You can uninstall cefpyco by running `make uninstall`.

## Configuration

This cefpyco package consists of the following files and directories.

* `README.md(.html)`: This document file.
* `LICENSE`: License file.
* `CmakeLists.txt`: Configuration file for cmake.
* `src`: Source codes:
  + `cefpyco/`: Main Python package for cefpyco (Python).
  + `cefpyco_wrap/`: C code for Cefore function calls (C-lang).
  + `cefpyco_c/`: Python/C wrapper code for cefpyco_c (C-lang).
* `cefapp`: Example applications for cefpyco.
* `test`: Test codes. You can run them via `make test` or `pytest`.
* `setup.py(.cfg)`, `pyproject.toml`: File containing configuration for cefpyco installation.

`CMakeLists.txt` exists in each directory as a configuration file for cmake.

The files in each directory are as follows.

### src/cefpyco

* `__init__.py`: File to be executed when importing a cefpyco package.
* `core.py`: File describing the main functions of cefpyco.

### src/cefpyco_wrap

* `pywrap_cefpyco.c`, `include/pywrap.h`: Python wrapper for Cefore that calls functions defined in cefpyco.c from Python programs.

### src/cefpyco_c

The source and header files are listed together, but the header files are under the `include` directory.

* `cefpyco.c(.h)`: Wraps Cefore functions and provides simplified functions.
* `cefpyco_parse.c(.h)`: Provides functions to parse TLVs received by the cef_client_read function.
* `cefpyco_def.h`: Definition of various constants.
* `cefpyco_util.c(.h)`: Utilities.
* `cefparse/cpcparse_tlv.c(.h)`: Provides functions for parsing TLV headers.
* `cefparse/cpcparse_type.c(.h)`: Definitions of the cpcparse_parse_info structure and related functions to hold information during parsing.
* `cefparse/cpcparse_app_frame_7_5.c(.h)`: Provides functions to parse received Data (`struct cef_app_frame`). Used in Cefore 0.7.5 or later versions.
* `cefparse/cpcparse_app_frame.c(.h)`: Provides functions to parse received Data (`struct cef_app_frame`). Used in versions earlier than Cefore 0.7.5.
* `cefparse/cpcparse_interest.c(.h)`: Provides functions to parse received Interest packets. Used in Cefore 0.7.5 or later versions.
* `cefparse/cpcparse_intreturn.c(.h)`: Provides functions to parse received Interest Return packets. Used in Cefore 0.8.2.2 or later versions.

### cefapp

`cefapp` is the Python application that can communicate using cefpyco.
`cefapp.py` provides the CefApp class for the communication with pipelines.

Please read the [README.md](./cefapp/README.md) of cefapp.

### test

* `test_cefpyco.c`: Code for automatic testing of features in the libcefpyco.so shared library.
* `test_libcefpycotest.py`: Code for automatic testing of features in the libcefpyco.so shared library.
* `test_core.py`: Code for automatic testing of cefpyco/core.py.
* `cefpycotest.py`: Code for manual testing of features in the libcefpyco.so shared library.

## How to use cefpyco

The following describes how to communicate with cefnetd using cefpyco.

### Connecting to and disconnecting from cefnetd

**The process from connection to disconnection can be implemented using `cefpyco.create_handle()`. **

```python
import cefpyco

with cefpyco.create_handle() as handle:
    # Write code.
    Pass: # Write code.
```

"handle" is an instance of the CefpycoHandle class, through which applications communicate. As in the case of file processing with the "with" block, at the beginning of the "with" block, the connection with Cefore is initialized, and at the end of the "with" block, the connection is terminated.

The `create_handle()` function has the following optional arguments.

* enable_log(default value: True): Enables/disables displaying of cefpyco's log.
* ceforedir(default value: None): Specifies the directory where Cefore configuration files such as cefnetd.conf are located.
    - If `None' is specified, the value of the environment variable `CEFORE_DIR` is used.
    - If the environment variable is also empty, the default installation directory `/usr/local/cefore` is used.
* portnum (default value: 9896): cefnetd port number to which the application connects.

Note: If you use the "with" block, you cannot use "handle" outside of that block. Additionally, unlike file handling,
**It is not possible to execute `cefpyco.create_handle()` again inside a "with" block** (singleton pattern).

If you do not want to use the "with" block, create a CefpycoHandle instance (see below), call the `begin()` method before your code, and call the `end()` method at the end.

````python
import cefpyco

CefpycoHandle()
handle.begin()
# Write code
handle.end()
````

### Sending Interest packets

The following is an example implementation of sending an Interest packet.

````python
import cefpyco

with cefpyco.create_handle() as h:
    h.send_interest("ccnx:/test", 0)
````

**With `h.send_interest(name, chunk_num)`, Interest packets are sent with the content `name` and `chunk_num`.**

If you want to receive multiple chunks, you need to send multiple Interests. For example, if you want to receive 100 chunks, do the following.

````python
import cefpyco

with cefpyco.create_handle() as h:
    for i in range(100):
        h.send_interest("ccnx:/test", i)
````

Note that this is only used for sending Interest packets. For receiving Data packets, see "Receiving packets" described below.

It also has the following keyword arguments.

* `hop_limit`: The maximum number of hops for an Interest.
* `lifetime`: Lifetime of an Interest (specified in milliseconds).

In addition, if you wish to use Symbolic Interest (SMI) instead of regular Interest, use
**`h.send_symbolic_interest(name)`**.
SMI has the following two characteristics.

1. The PIT entry is not deleted when cefnetd receives Data, only after timeout.
2. Any chunk data can be received if a name prefix without a chunk number is specified.
    * Example: If you request `ccnx:/a` in SMI, you will receive all chunks named `ccnx:/a/Chunk=0`, `ccnx:/a/Chunk=1`, ..., etc.

This is mainly useful for real-time video streaming.

### Sending Data packets

The following is an example implementation of the process of sending Data packets.

````python
import cefpyco

with cefpyco.create_handle() as h:
    h.send_data("ccnx:/test", "hello", 0)
````

**The `h.send_data(name, payload, chunk_num)` method requires that the content `name`, `chunk_num`, and payload(data in cob) be specified. **
The payload type can be byte or string.

As with the Interest packet, this is a method that only sends Data packets. For responding to an Interest packet after receiving it, see "Receiving packets" below.

It also has the following keyword arguments.

* `end_chunk_num`: End chunk number of the content. If the Producer knows the number of chunks of the entire content in advance, this is used to inform the Consumer of the chunk number to request.
* `hop_limit`: Maximum number of hops.
* `expiry`: Expiry time of the content. The relative time from the current time in milliseconds.
* `cache_time`: Recommended cache time for the content. The relative time from the current time in milliseconds.


### Receiving packets

**The method for receiving a packet is `h.receive()`, and it returns the CcnPacketInfo object. **

The following is an example implementation, which waits for Data reception after sending an Interest (it also shows the details of the last received Data).

````python
import cefpyco

with cefpyco.create_handle() as h:
    h.send_interest("ccnx:/test", 0)
    info = h.receive()
    print("info.is_succeeded:{0}".format(info.is_succeeded))
    print("info.is_failed:{0}".format(info.is_failed))
    print("info.version:{0}".format(info.version))
    print("info.type:{0}".format(info.type))
    print("info.actual_data_len:{0}".format(info.actual_data_len))
    print("info.name:{0}".format(info.name))
    print("info.name_len:{0}".format(info.name_len))
    print("info.chunk_num:{0}".format(info.chunk_num))
    print("info.end_chunk_num:{0}".format(info.end_chunk_num))
    print("info.payload:{0}".format(info.payload))
    print("info.payload_s:{0}".format(info.payload_s))
    print("info.payload_len:{0}".format(info.payload_len))
````

The **receive method has the optional argument `error_on_timeout` (default value: False).
It waits for about 4 seconds after the method is executed and returns the process if it cannot receive the message. **
If you want to receive packets until a successful packet reception, you need to use a loop syntax such as "while."
In this case, if error_on_timeout is set to "True," CefpycoHandle disconnects from Cefore. (It cannot receive anything anymore.)

The receive method returns the received result in a CcnPacketInfo object, regardless of whether the received packet is an Interest or Data packet. The meaning of each property value of this object is as follows.

* is_succeeded/is_failed: Success/failure flag of packet reception.
* version: Version value of the received packet.
* type: Type value of the received packet, which can distinguish Data/Interest/InterestReturn.
* returncode: ReturnCode value if InterestReturn was returned (see [RFC8609] (https://datatracker.ietf.org/doc/html/rfc8609#section-4.2)).
* actual_data_len: Byte length of the received packet (or data) including its header.
* name: Name expressed in string type.
* name_len: Length of the name.
* chunk_num: Number of chunks.
* end_chunk_num: Last chunk number of the content (available only when you set it at the data producer app.).
* payload: Byte sequence of the payload (displayed as `(empty)` if the value is empty); returns "str" in Python 2, "bytes" in Python 3.
* payload_len: Length of the payload. If it is empty, it is set to 0.

The CcnPacketInfo object also has the `is_interest`, `is_data`, and `is_interest_return` (or `is_return`) properties.
These can be used to distinguish which type of CCN packet the received packet is.
For example, in the case of a Consumer app that sends Interests and receives Data, it is possible to receive InterestReturn as well as Data. (For example, if cefnetd has no upstream route configured, it will receive an InterestReturn indicating `No Route`. Therefore, it is necessary to distinguish such received packets.)

When an Interest is received, to determine whether it is an SMI or not, the `is_symbolic` property can be used.
The `is_regular_interest` and `is_symbolic_interest` properties are used to distinguish a regular Interest from an SMI.

The following points should be noted.

* The receive method does not wait for all packets received by cefnetd.
    - To receive Interest packets in your app, you need to call the "register" method.
    - In order to receive Data packets in your app, Interest packets must be sent first.
* Because the cefnetd buffer cannot store many packets, the receive method must be called at appropriate intervals.
    We recommend that you implement such that the application receives packets from cefnetd by calling the receive method at appropriate intervals.
* Data packets are inserted into a structure prepared for Cefore apps and sent to the app; therefore, `actual_data_len` may not necessarily match the length of the CCNx packet format specification.

### Creating Publisher App

Using the previous functions, we can create a Publisher app (an app that listens for Interests and then sends back Data packets). The following example implements a Publisher app.

````python
import cefpyco

with cefpyco.create_handle() as h:
    h.register("ccnx:/test")
    while True:
        info = h.receive()
        if info.is_success and info.name == "ccnx:/test" and info.chunk_num == 0
            h.send_data("ccnx:/test", "hello", 0)
            # break # Uncomment if publisher provides content once
````

**To wait for Interest packets, the `h.register()` method is used. **
This method registers the prefix of the Interest that you want to receive from cefnetd.
For example, in addition to `ccnx:/test`, you can receive Interests with a name such as `ccnx:/test/foo/bar.txt`.

Then, by using the "while loop" syntax, the Publisher app continues to execute `h.receive()`.
If the target Interest is successfully received, `h.send_data()` is called and a Data packet will be sent back.
Because this is a Publisher app, it will keep looping indefinitely. If you want to provide the content just once, you can break the loop by inserting `break`.

### Creating Consumer app

The following example implements a Consumer app (that waits for Data after sending an Interest).

````python
from time import sleep
import cefpyco

with cefpyco.create_handle() as h:
    while True:
        h.send_interest("ccnx:/test", 0)
        info = h.receive()
        if info.is_success and info.name == "ccnx:/test" and info.chunk_num == 0
            print("Success")
            print(info)
            break # Uncomment if publisher provides content once
        sleep(1)
````

**If you send Interests, you can then receive Data with the receive method. **
If the Data is successfully received, print ``Success`` and exit.
To consider the case that InterestReturn is returned if the content cannot be returned, `info.is_data` should be added to the condition.

In this example, the receive method loops until Data is received. Note that if the interval for sending Interests is too short, Interest packets will be aggregated into PIT; therefore, one chunk is `sleep(1)` and the other chunk is `data`.

In this example, the loop exits after one chunk is received. It is also possible to receive content consisting of multiple chunks by changing each chunk number. If you wish to receive content consisting of many chunks, you can send multiple Interests concurrently for high-speed communication. However, in such a case, you must make preparations for the sending of a large number of Interests simultaneously (due to the PIT size configured in cefnetd.conf and the limitation of the processing performance). It is recommended that you implement pipeline processing for such cases.

## Notes

* This tool was developed to simplify application development using Cefore. Bug reports are welcome.
    - It has only been confirmed to work on Ubuntu 16.04 and Raspberry Pi (as of cefpyco 0.2.0).
    - It has also only been confirmed to work with Python 3.
    - Bug reports can be sent to GitHub (https://github.com/cefore/cefore/issues).
* If cefnetd is started with sudo privileges, sudo may be required even when using Python tools. If the tool fails to start, check the startup log to see if `Permission Denied` or a similar message is present. If necessary, run the Python tool with sudo privileges.
* If you terminate cefnetd using commands such as "kill" without using cefnetdstop, you will get the following error message:
    "Another cefnetd is already listening on this port," and subsequent startup may fail.
    * The reason is that the socket file is still in the directory `/tmp`.
    * In such a case, try `sudo cefnetdstop -F`, which will execute a forced termination, including deletion of the socket file.
* If the application cannot receive Interests, check if there are some matching entries for the Interest name in the FIB. 
    - In particular, if an entry for `ccnx:/` is registered in the FIB, that entry may be prioritized (as of Cefore 0.7.2).
    - From Cefore 0.8.0, the specification has been changed so that the application preferentially receives the Interest.
* If communication fails, check if buffer tuning is performed before starting cefnetd.
    - For Linux OS, execute the following.
    ```
    $ _cef_mem_size=10000000
    $ sudo sysctl -w net.core.rmem_default=$_cef_mem_size
    $ sudo sysctl -w net.core.wmem_default=$_cef_mem_size
    $ sudo sysctl -w net.core.rmem_max=$_cef_mem_size
    $ sudo sysctl -w net.core.wmem_max=$_cef_mem_size
    ````
    - For Mac OS, do the following.
    ```
    $ sudo sysctl -w net.local.stream.sendspace=2000000
    $ sudo sysctl -w net.local.stream.recvspace=2000000
    ````
    - Using a larger buffer is recommended, especially for executing high-speed communication and/or dealing with large files.
    - Since this setting is initialized every time the PC is rebooted, ensure that measures are put in place to prevent forgetting to execute it. The configuration should be automatically executed at PC startup.
* The default value for Interest lifetime is 4 seconds, and the default value for Data cache time (i.e., cache survival time) is 10 hours.
    - If you wish to change them, change the values in the corresponding parts of cefpyco.c.
* After executing cmake, copying the entire directory to another location and trying to execute cmake again will fail.
    - If you wish to run cmake again, you need to delete the CMakeFiles directory, CMakeCache.txt, and cmake_install.cmake manually.
    - Commands such as `cmake clean` do not exist.
