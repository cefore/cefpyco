# Cefore Python Compact package: cefpyco

- [Cefore Python Compact package: cefpyco](#cefore-python-compact-package-cefpyco)
  - [Overview](#Overview)
  - [Installation](#Installation)
    - [Installing Cefore](#Installing_Cefore)
    - [Installing Cefpyco](#Installing_Cefpyco)
    - [Uninstalling Cefpyco](#Uninstalling_Cefpyco)
  - [Configuration](#Configuration)
    - [cefpyco](#cefpyco)
    - [cefapp](#cefapp)
    - [c_src](#c_src)
    - [test](#test)
  - [How to use cefpyco](#cefpyco)
    - [Connecting to and disconnecting from cefnetd](#Connecting_to_and_disconnecting_from_cefnetd)
    - [Sending Interest Packets](#Sending_Interest_Packets)
    - [Sending Data Packets](#Sending_Data_Packets)
    - [Receiving Packets](#Receiving_Packets)
    - [Creating Publisher App](#Creating_Publisher_App)
    - [Creating Consumer App](#Creating_Consumer_App)
  - [How to use CefApp](#How_to_use_cefapp)
    - [Overview](#Overview-1)
    - [cefappconsumer.py](#cefappconsumerpy)
    - [cefappproducer.py](#cefappproducerpy)
    - [example communication](#example communication)
  - [Notes](#Cautions)
  - [Credit](#credit)

## Overview

cefpyco is a Python package for Cefore applications.
It has the following features.

* Cefore functions, which are written in C, can be called from Python programs.
  * It is easier to develop Cefore applications in Python than in C.
* Interest and Data transmission/reception can be executed with a single function.
* Support for “with” blocks, enabling the easy programming of initialization and termination of connections with cefnetd.
* A simple application that sends back Data in response to receiving an Interest can be easily developed.
  * This means content can be provided without accessing csmgrd.
* Optional TLVs are supported (e.g., chunk number, termination chunk number, maximum hop count, Interest lifetime, content expiration, and cache expiration).
* InterestReturn packets are supported.
  * InterestReturn is an Interest packet returned only when an Interest packet is lost (it works like NACK).
  * Note that InterestReturn with "No Route" is returned when the upstream router or FIB is not appropriately set.

## Installation

### Installing_Cefore	

First, install Cefore on your PC (target version: 0.8.3 or later).
To install Cefore, visit the Cefore website (https://cefore.net) or GitHub (https://github.com/cefore/cefore).
(The user manual is available at https://cefore.net/instruction and the source code is available at https://cefore.net/download).

### Installing_Cefpyco

Execute the following commands in an environment where Cefore and Python are installed.
(Do not forget to add "." with cmake).

```
sudo apt-get install cmake python3-pip
pip3 install setuptools click numpy
cmake .
sudo make install
````

After executing the above commands, you can `import cefpyco` from any directory.

For Python packages, setuptools is required for installation, but click and numpy are not necessary if you do not use cefapp.

### Uninstalling_Cefpyco

You can uninstall cefpyco by running `sudo make uninstall`.

## Configuration

This cefpyco package consists of the following files and directories.

* `README.md(.html)`: This document file.
* `LICENSE.md(.html)`: License file.
* `CmakeLists.txt`: Configuration file for cmake.
* `cefpyco/`: Main Python package for cefpyco.
* `cefapp/`: Example applications for cefpyco.
* `c_src/`: C code for Cefore function calls.
* `setup.py`: File containing configuration for cefpyco installation.

The files in each directory are as follows.

### cefpyco

* `__init__.py`: File to be executed when importing a cefpyco package.
* `core.py`: File describing the main functions of cefpyco.

### cefapp

* `cefapp.py`: File that provides the CefApp class for applications that can communicate with pipelines using cefpyco.
* `cefappproducer.py`: Consumer application using CefApp.
* `cefappconsumer.py`: Producer application using CefApp.

### c_src

* `pywrap_cefpyco.c`: Python wrapper for Cefore that calls functions defined in cefpyco.c from Python programs.
* `cefpyco.c(.h)`: Wraps Cefore functions and provides simplified functions.
* `cefpyco_parse.c(.h)`: Provides functions to parse TLVs received by the cef_client_read function.
* `cefpyco_def.h`: Definition of various constants.
* `cefpyco_util.c(.h)`: Utilities.
* `cefparse/cpcparse_tlv.c(.h)`: Provides functions for parsing TLV headers.
* `cefparse/cpcparse_type.c(.h)`: Definitions of the cpcparse_parse_info structure and related functions to hold information during parsing.
* `cefparse/cpcparse_app_frame_7_5.c(.h)`: Provides functions to parse received Data (`struct cef_app_frame`). Used in Cefore 0.7.5 or later versions.
* `cefparse/cpcparse_app_frame.c(.h)`: Provides functions to parse received Data (`struct cef_app_frame`). Used in versions earlier than Cefore 0.7.5.
* `cefparse/cpcparse_interest.c(.h)`: Provides functions to parse received Interest packets. Used in Cefore 0.7.5 or later versions.

`CMakeLists.txt` exists in each directory as a configuration file for cmake.

### test

* `cefpycotest.py`: Code for manual testing of features in the libcefpyco.so shared library.
* `test_libcefpycotest.py`: Code for automatic testing of features in the libcefpyco.so shared library.
* `test_core.py`: Code for automatic testing of cefpyco/core.py.

## How to use cefpyco

The following describes how to communicate with cefnetd using cefpyco.

### Connecting_to_and_disconnecting_from_cefnetd

**The process from connection to disconnection can be implemented using `cefpyco.create_handle()`. **

```python
import cefpyco

with cefpyco.create_handle() as handle:
    # Write code.
    Pass: # Write code.
````

“handle” is an instance of the CefpycoHandle class, through which applications communicate. As in the case of file processing with the “with” block, at the beginning of the “with” block, the connection with Cefore is initialized, and at the end of the “with” block, the connection is terminated.

The `create_handle()` function has the following optional arguments.

* enable_log(default value: True): Enables/disables displaying of cefpyco's log.
* ceforedir(default value: None): Specifies the directory where Cefore configuration files such as cefnetd.conf are located.
    - If `None' is specified, the value of the environment variable `CEFORE_DIR` is used.
    - If the environment variable is also empty, the default installation directory `/usr/local/cefore` is used.
* portnum (default value: 9896): cefnetd port number to which the application connects.

Note: If you use the “with” block, you cannot use “handle” outside of that block. Additionally, unlike file handling,
**It is not possible to execute `cefpyco.create_handle()` again inside a “with” block** (singleton pattern).

If you do not want to use the “with” block, create a CefpycoHandle instance (see below), call the `begin()` method before your code, and call the `end()` method at the end.

````python
import cefpyco

CefpycoHandle()
handle.begin()
# Write code
handle.end()
````

### Sending_Interest_packets

The following is an example implementation of sending an Interest packet.

````python
import cefpyco

with cefpyco.create_handle() as h:
    h.send_interest("ccnx:/test", 0)
```

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
(1) The PIT entry is not deleted when cefnetd receives Data, only after timeout.
(2) Any chunk data can be received if a name prefix without a chunk number is specified.
(Example: If you request `ccnx:/a` in SMI, you will receive all chunks named `ccnx:/a/Chunk=0`, `ccnx:/a/Chunk=1`, ..., etc.).
This is mainly useful for real-time video streaming.

### Sending_Data_packets

The following is an example implementation of the process of sending Data packets.

````python
import cefpyco

with cefpyco.create_handle() as h:
    h.send_data("ccnx:/test", "hello", 0)
```

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

```python
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
If you want to receive packets until a successful packet reception, you need to use a loop syntax such as “while.”
In this case, if error_on_timeout is set to “True,” CefpycoHandle disconnects from Cefore. (It cannot receive anything anymore.)

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
* payload: Byte sequence of the payload (displayed as `(empty)` if the value is empty); returns “str” in Python 2, “bytes” in Python 3.
* payload_len: Length of the payload. If it is empty, it is set to 0.

The CcnPacketInfo object also has the `is_interest`, `is_data`, and `is_interest_return` (or `is_return`) properties.
These can be used to distinguish which type of CCN packet the received packet is.
For example, in the case of a Consumer app that sends Interests and receives Data, it is possible to receive InterestReturn as well as Data. (For example, if cefnetd has no upstream route configured, it will receive an InterestReturn indicating `No Route`. Therefore, it is necessary to distinguish such received packets.)

When an Interest is received, to determine whether it is an SMI or not, the `is_symbolic` property can be used.
The `is_regular_interest` and `is_symbolic_interest` properties are used to distinguish a regular Interest from an SMI.

The following points should be noted.

* The receive method does not wait for all packets received by cefnetd.
    - To receive Interest packets in your app, you need to call the “register” method.
    - In order to receive Data packets in your app, Interest packets must be sent first.
* Because the cefnetd buffer cannot store many packets, the receive method must be called at appropriate intervals.
    We recommend that you implement such that the application receives packets from cefnetd by calling the receive method at appropriate intervals.
* Data packets are inserted into a structure prepared for Cefore apps and sent to the app; therefore, `actual_data_len` may not necessarily match the length of the CCNx packet format specification.

### Creating_Publisher_App

Using the previous functions, we can create a Publisher app (an app that listens for Interests and then sends back Data packets). The following example implements a Publisher app.

```python
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

Then, by using the “while loop” syntax, the Publisher app continues to execute `h.receive()`.
If the target Interest is successfully received, `h.send_data()` is called and a Data packet will be sent back.
Because this is a Publisher app, it will keep looping indefinitely. If you want to provide the content just once, you can break the loop by inserting `break'.

### Creating_Consumer_app

The following example implements a Consumer app (that waits for Data after sending an Interest).

```python
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

## How_to_use_CefApp

The following describes the cefappconsumer and cefappproducer tools, which are content request and provision tools, respectively, implemented using CefpycoHandle.

### Overview

The cefappconsumer and cefappproducer tools have the following characteristics.

* cefappconsumer sends out an Interest and receives Data, while cefappproducer listens for an Interest and sends Data.
* Three types of input/output can be selected: inline, standard input/output, and file.
* cefappconsumer implements pipeline processing and can communicate with cefappproducer at high speed.
    - As a reference, approximately 80 Mbps was observed in a simple environment.

When cefappconsumer and cefappproducer are launched after cefnetd is started, cefappconsumer operates to obtain content from cefappproducer.
The order in which cefappconsumer and cefappproducer are invoked is irrelevant, but the other must be started before either one times out (within about 8 seconds, which is the default timeout setting).

As a CefApp-specific process, in order to share content cob count information between cefappconsumer and cefappproducer, cefappconsumer requests content with `/meta` at the end of the content name to cefappproducer, and obtains the number of cobs in advance. Therefore, at least for consumers, note that it cannot be used in combination with other data-provisioning tools (e.g., cefputfile). (The combination of cefappproducer and cefgetfile can be used.)

Since July 2021, Cefore has supported end chunk numbers, which makes the above trick unnecessary. However, because it is also useful for exchanging information other than cob counts in advance, implementation of the above metafile exchange method is retained.


### cefappconsumer.py

* Usage.
    ```
    cefappconsumer.py [OPTIONS] name
    ````
* Summary
    - Requests a content specified by ``name'' and outputs the information contained in the received content.
* Options.
    - `[-t|--timeout int]`: Specifies the number of times Consumer will tolerate Data request failures. One request will wait for about 4 seconds. Default is two times.
    - `[-p|--pipeline int]`: Number of pipelines. Too many pipelines may cause PIT overflow or exceed cefnetd's processing limit, etc. Pay attention to the PIT size limitation specified by cefnetd.conf and processing performance limitation. Default is 10.
    - `[-f|--filename str]`: Specifies a filename to use in file mode (see the `-o` option). Even if you do not explicitly set file mode with the `-o` option, if you specify a filename here, it is treated as file mode. By default, the last segment name of ``name” is used.
    - `[-o|--output mode]`: Specifies the output mode. “mode” can be one of the following strings (default is stdout mode).
        - none: No output mode. No data is output anywhere (lightweight, because it is not stored in the internal buffer).
        - stdout: Standard output mode. Outputs the contents of received data using the standard output of a terminal or other device.
        - file: File output mode. This mode outputs the received contents to a file whose name is specified by filename or the last segment name of ``name’’.
    - `[-q|--quiet]`: If specified, no log output.
* Example usage
    - `. /cefappconsumer.py ccnx:/test`.
        - Receive content named `ccnx:/test` and output the received content to standard output.
    - `. /cefappconsumer.py ccnx:/test/a -o file`.
        - Receive content named `cccnx:/test/a' and output the received content to a file named `a`.
    - `. /cefappconsumer.py ccnx:/test/a -o file -f b`
        - Receive content named ccnx:/test/a and output the received content to a file named `b`.
    - `. /cefappconsumer.py ccnx:/test -o none -q`
        - Only communicate to receive ccnx:/test; do not output any log or received content.

### cefappproducer.py

* Usage.
    ```
    cefappproducer.py [OPTIONS] name [arg].
    ````
* Summary
    - Create Data from the contents specified by ``arg'' etc., and provide Data with the content name specified by ``name''.
* Options.
    - `[-t|--timeout int]`: Specifies the number of times Producer will tolerate Data wait failures. One wait process will wait for about 4 seconds. Default is two times.
    - `[-b|--block_size int]`: Size of one content object. Default is 1024 bytes. Use with care for MTU, since there is no constraint on minimum and maximum values.
    - `[-i|--input]`: Specifies the input mode. “mode” can be one of the following strings (default is arg mode).
        - arg: Inline mode. The content is written directly in the argument “arg”.
        - stdin: Standard input mode. Content is created from standard input.
        - file: File input mode. It creates a content from a file whose name is the last segment name of “name” or the file name specified in the argument “arg”.
    - `[-q|--quiet]`: If specified, no log is output.
* Example usage.
    - `. /cefappproducer.py ccnx:/test helloworld`
        - Create and serve a 10-character content named `helloworld` with the name ccnx:/test.
    - `. /cefappproducer.py ccnx:/test helloworld -b 5`
        - Create and serve a content named ccnx:/test for every five characters (i.e., a content `hello` and a content `world`)
            (`hello` is provided as the data chunk number 0 and `world` as the data chunk number 1).
    - `. /cefappproducer.py ccnx:/test/a -i file`.
        - Create a content from a file named `a` and serve it under the name ccnx:/test/a.
    - `. /cefappproducer.py ccnx:/test/a b -i file -o none -q`
        - Create a content from a file named `b` and serve it under the name ccnx:/test/a.


### Communications example

Below is an example of communication in which cefappproducer publishes the string "hello" as a Data packet with the name `ccnx:/test`, and cefappconsumer fetches it.

The following shows that at terminal t1, cefappproducer is started with cefnetd running, and at terminal 2, cefappconsumer is started with cefnetd running. (You can check the available options for each tool with the `--help` option.)

```
t1$ sudo . /cefappproducer.py ccnx:/test hello
[cefpyco] Configure directory is /usr/local/cefore
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Config directory is /usr/local/cefore
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Local Socket Name is /tmp/cef_9896.0
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Listen Port is 9896
[cefapp] Receiving Interest...
t2$ sudo . /cefappconsumer.py ccnx:/test
[cefpyco] Configure directory is /usr/local/cefore
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Config directory is /usr/local/cefore
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Local Socket Name is /tmp/cef_9896.0
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Listen Port is 9896
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: Send interest (name: ccnx:/test/meta, #chunk: 0)
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: Send interest (name: ccnx:/test, #chunk: 0)
[cefapp] Succeed to receive.
hello
````

## Notes.

* This tool was developed to simplify application development using Cefore. Bug reports are welcome.
    - It has only been confirmed to work on Ubuntu 16.04 and Raspberry Pi (as of cefpyco 0.2.0).
    - It has also only been confirmed to work with Python 3.
    - Bug reports can be sent to GitHub (https://github.com/cefore/cefore/issues).
* If cefnetd is started with sudo privileges, sudo may be required even when using Python tools. If the tool fails to start, check the startup log to see if `Permission Denied` or a similar message is present. If necessary, run the Python tool with sudo privileges.
* If you terminate cefnetd using commands such as “kill” without using cefnetdstop, you will get the following error message:
    “Another cefnetd is already listening on this port,” and subsequent startup may fail.
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
