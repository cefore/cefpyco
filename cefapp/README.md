# CefApp: Cefore applications using cefpyco

- [CefApp: Cefore applications using cefpyco](#cefapp-cefore-applications-using-cefpyco)
  - [Overview](#overview)
  - [cefapp consumer](#cefapp-consumer)
  - [cefapp producer](#cefapp-producer)
  - [Example](#example)


## Overview

`cefapp` is the Python application that can communicate using cefpyco.
You can install it with pyproject.toml via the following commands:

```bash
python -m build
pip install dist/cefapp-x.x.x.tar.gz
```

After installing cefapp, you can use the following two subcommands: 
`cefapp producer` and `cefapp consumer`, 
which are content request and provision tools, respectively, 
implemented using CefpycoHandle.
The `cefapp producer` and `cefapp consumer` tools have the following characteristics.

* `cefapp consumer` sends out an Interest and receives Data, while `cefapp producer` listens for an Interest and sends Data.
* Three types of input/output can be selected: inline, standard input/output, and file.
* `cefapp producer` implements pipeline processing and can communicate with `cefapp consumer` at high speed.
    - As a reference, approximately 80 Mbps was observed in a simple environment.

When `cefapp consumer` and `cefapp producer` are launched after cefnetd is started, `cefapp consumer` operates to obtain content from `cefapp producer`.
The order in which `cefapp consumer` and `cefapp producer` are invoked is irrelevant, but the other must be started before either one times out (within about 8 seconds, which is the default timeout setting).

As a CefApp-specific process, in order to share content cob count information between `cefapp consumer` and `cefapp producer`, `cefapp consumer` requests content with `/meta` at the end of the content name to `cefapp producer`, and obtains the number of cobs in advance. Therefore, at least for consumers, note that it cannot be used in combination with other data-provisioning tools (e.g., cefputfile). (The combination of `cefapp producer` and cefgetfile can be used.)

Since July 2021, Cefore has supported end chunk numbers, which makes the above trick unnecessary. However, because it is also useful for exchanging information other than cob counts in advance, implementation of the above metafile exchange method is retained.


## cefapp consumer

* Usage.
    ```
    cefapp consumer [OPTIONS] name
    ````
* Summary
    - Requests a content specified by ``name'' and outputs the information contained in the received content.
* Options.
    - `[-t|--timeout int]`: Specifies the number of times Consumer will tolerate Data request failures. One request will wait for about 4 seconds. Default is two times.
    - `[-p|--pipeline int]`: Number of pipelines. Too many pipelines may cause PIT overflow or exceed cefnetd's processing limit, etc. Pay attention to the PIT size limitation specified by cefnetd.conf and processing performance limitation. Default is 10.
    - `[-f|--filename str]`: Specifies a filename to use in file mode (see the `-o` option). Even if you do not explicitly set file mode with the `-o` option, if you specify a filename here, it is treated as file mode. By default, the last segment name of ``name" is used.
    - `[-o|--output mode]`: Specifies the output mode. "mode" can be one of the following strings (default is stdout mode).
        - none: No output mode. No data is output anywhere (lightweight, because it is not stored in the internal buffer).
        - stdout: Standard output mode. Outputs the contents of received data using the standard output of a terminal or other device.
        - file: File output mode. This mode outputs the received contents to a file whose name is specified by filename or the last segment name of ``name’’.
    - `[-q|--quiet]`: If specified, no log output.
* Example usage
    - `cefapp consumer ccnx:/test`.
        - Receive content named `ccnx:/test` and output the received content to standard output.
    - `cefapp consumer ccnx:/test/a -o file`.
        - Receive content named `cccnx:/test/a' and output the received content to a file named `a`.
    - `cefapp consumer ccnx:/test/a -o file -f b`
        - Receive content named ccnx:/test/a and output the received content to a file named `b`.
    - `cefapp consumer ccnx:/test -o none -q`
        - Only communicate to receive ccnx:/test; do not output any log or received content.

## cefapp producer

* Usage.
    ```
    cefapp producer [OPTIONS] name [arg].
    ````
* Summary
    - Create Data from the contents specified by ``arg'' etc., and provide Data with the content name specified by ``name''.
* Options.
    - `[-t|--timeout int]`: Specifies the number of times Producer will tolerate Data wait failures. One wait process will wait for about 4 seconds. Default is two times.
    - `[-b|--block_size int]`: Size of one content object. Default is 1024 bytes. Use with care for MTU, since there is no constraint on minimum and maximum values.
    - `[-i|--input]`: Specifies the input mode. "mode" can be one of the following strings (default is arg mode).
        - arg: Inline mode. The content is written directly in the argument "arg".
        - stdin: Standard input mode. Content is created from standard input.
        - file: File input mode. It creates a content from a file whose name is the last segment name of "name" or the file name specified in the argument "arg".
    - `[-q|--quiet]`: If specified, no log is output.
* Example usage.
    - `cefapp producer ccnx:/test helloworld`
        - Create and serve a 10-character content named `helloworld` with the name ccnx:/test.
    - `cefapp producer ccnx:/test helloworld -b 5`
        - Create and serve a content named ccnx:/test for every five characters (i.e., a content `hello` and a content `world`)
            (`hello` is provided as the data chunk number 0 and `world` as the data chunk number 1).
    - `cefapp producer ccnx:/test/a -i file`.
        - Create a content from a file named `a` and serve it under the name ccnx:/test/a.
    - `cefapp producer ccnx:/test/a b -i file -o none -q`
        - Create a content from a file named `b` and serve it under the name ccnx:/test/a.


## Example

Below is an example of communication in which `cefapp producer` publishes the string "hello" as a Data packet with the name `ccnx:/test`, and `cefapp consumer` fetches it.

The following shows that at terminal t1, `cefapp producer` is started with cefnetd running, and at terminal 2, `cefapp consumer` is started with cefnetd running. (You can check the available options for each tool with the `--help` option.)

```bash
t1$ cefapp producer ccnx:/test hello
[cefpyco] Configure directory is /usr/local/cefore
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Config directory is /usr/local/cefore
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Local Socket Name is /tmp/cef_9896.0
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Listen Port is 9896
[cefapp] Receiving Interest...
t2$ cefapp consumer ccnx:/test
[cefpyco] Configure directory is /usr/local/cefore
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Config directory is /usr/local/cefore
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Local Socket Name is /tmp/cef_9896.0
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Listen Port is 9896
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: Send interest (name: ccnx:/test/meta, #chunk: 0)
YYYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: Send interest (name: ccnx:/test, #chunk: 0)
[cefapp] Succeed to receive.
hello
```
