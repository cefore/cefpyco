#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2016--2023, National Institute of Information and Communications
# Technology (NICT). All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the NICT nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE NICT AND CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE NICT OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

import sys
import click
import cefpyco
import logging
from rich import print
from rich.console import Console
from rich.logging import RichHandler
from rich.traceback import install as _rich_traceback_install
from cefapp import CefAppConsumer
from cefapp import MetaInfoNotResolvedError
from cefapp import CefAppProducer

_rich_traceback_install()

_logger_name = f"{__name__}"
_log_level = logging.INFO
_log_fmt = "[%(name)10s] %(message)s"
_log_datefmt = "[%X]"

log = logging.getLogger(_logger_name)
log.setLevel(_log_level)
_log_fmt = logging.Formatter(fmt=_log_fmt, datefmt=_log_datefmt)
_log_hdl = RichHandler(rich_tracebacks=True, console=Console(stderr=True))
_log_hdl.setFormatter(_log_fmt)
log.addHandler(_log_hdl)


@click.group()
def cmd():
    pass


@cmd.command()
@click.argument("name")
@click.option(
    "--timeout",
    "-t",
    default=2,
    help=(
        "How many times consumer tries to receive Data "
        "(one receiving action takes about 4 seconds)."
    ),
)
@click.option("--pipeline", "-s", default=10, help="Number of pipeline.")
@click.option(
    "--filename",
    "-f",
    default="",
    help="Name of output file (in mode [file], you must specify)",
)
@click.option(
    "--output",
    "-o",
    type=click.Choice(["none", "stdout", "file"]),
    default="stdout",
    help=(
        "Output mode: "
        "[none] Not output. [stdout] Output to stdout. [file] Output to file."
    ),
)
@click.option("--debug", "-g", is_flag=True, help="Enable debug flag.")
@click.option("--quiet", "-q", is_flag=True, help="Enable quiet flag.")
def consumer(name, timeout, pipeline, filename, output, debug, quiet):
    data_store = output != "none"
    enb_log = not quiet
    if debug:
        log.setLevel(logging.DEBUG)
    with cefpyco.create_handle(enable_log=enb_log) as h:
        app = CefAppConsumer(
            h,
            timeout_limit=timeout,
            pipeline=pipeline,
            data_store=data_store,
            enable_log=enb_log,
        )
        try:
            app.run(name)
        except MetaInfoNotResolvedError as e:
            return
        if filename or output == "file":
            with open(filename or name.split("/")[-1], "w") as f:
                f.write(app.data)
        elif output == "stdout":
            print(app.data)


@cmd.command()
@click.argument("name")
@click.argument("arg", default="")
@click.option(
    "--timeout",
    "-t",
    default=2,
    help=(
        "How many times producer tries to receive Interest "
        "(one receiving action takes about 4 seconds)."
    ),
)
@click.option("--block_size", "-b", default=1024, help="Size of content object.")
@click.option(
    "--input",
    "-i",
    type=click.Choice(["arg", "stdin", "file"]),
    default="arg",
    help=(
        "Input mode: "
        "[arg] Inline mode. [stdin] Input from stdin. [file] Input from file."
    ),
)
@click.option("--debug", "-g", is_flag=True, help="Enable debug flag.")
@click.option("--quiet", "-q", is_flag=True, help="Enable quiet flag.")
def producer(name, arg, timeout, block_size, input, debug, quiet):
    enb_log = not quiet
    if debug:
        log.setLevel(logging.DEBUG)
    if input == "arg":
        data = arg
    elif input == "stdin":
        data = sys.stdin.read()
    elif input == "file":
        with open(arg or name.split("/")[-1], "r") as f:
            data = f.read()
    else:
        log.error("Invalid argument")
        return
    with cefpyco.create_handle(enable_log=enb_log) as h:
        app = CefAppProducer(
            h, timeout_limit=timeout, data=data, cob_len=block_size, enable_log=enb_log
        )
        app.run(name)


def main():
    cmd()


if __name__ == "__main__":
    main()
