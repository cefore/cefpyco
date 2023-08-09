#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2018, National Institute of Information and Communications
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
from cefapp import CefAppConsumer, MetaInfoNotResolvedError

@click.command()
@click.argument("name")
@click.option("--timeout", "-t", default=2,
    help=("How many times consumer tries to receive Data "
    "(one receiving action takes about 4 seconds)."))
@click.option("--pipeline", "-s", default=10, help="Number of pipeline.")
@click.option("--filename", "-f", default="", 
    help="Name of output file (in mode [file], you must specify)")
@click.option("--output", "-o", 
    type=click.Choice(["none", "stdout", "file"]), default="stdout",
    help=("Output mode: "
    "[none] Not output. [stdout] Output to stdout. [file] Output to file."))
@click.option("--quiet", "-q", is_flag=True, help="Enable quiet flag.")
def main(name, timeout, pipeline, filename, output, quiet):
    data_store = (output != "none")
    enb_log = not quiet
    with cefpyco.create_handle(enable_log=enb_log) as h:
        app = CefAppConsumer(
            h, 
            timeout_limit=timeout, 
            pipeline=pipeline, 
            data_store=data_store,
            enable_log=enb_log)
        try:
            app.run(name)
        except MetaInfoNotResolvedError as e:
            return
        if filename or output == "file":
            with open(filename or name.split("/")[-1], "w") as f:
                f.write(app.data)
        elif output == "stdout":
            print(app.data)
            
if __name__ == '__main__':
    main()
