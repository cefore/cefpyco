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
from cefapp import CefAppProducer

@click.command()
@click.argument("name")
@click.argument("arg", default="")
@click.option("--timeout", "-t", default=2,
    help=("How many times producer tries to receive Interest "
    "(one receiving action takes about 4 seconds)."))
@click.option("--block_size", "-b", default=1024, 
    help="Size of content object.")
@click.option("--input", "-i",
    type=click.Choice(["arg", "stdin", "file"]), default="arg",
    help=("Input mode: "
    "[arg] Inline mode. [stdin] Input from stdin. [file] Input from file."))
@click.option("--quiet", "-q", is_flag=True, help="Enable quiet flag.")
def main(name, arg, timeout, block_size, input, quiet):
    enb_log = not quiet
    if input == "arg":
        data = arg
    elif input == "stdin":
        data = sys.stdin.read()
    elif input == "file":
        with open(arg or name.split("/")[-1], "r") as f:
            data = f.read()
    else:
        print("Invalid argument")
        return
    with cefpyco.create_handle(enable_log=enb_log) as h:
        app = CefAppProducer(
            h, 
            timeout_limit=timeout, 
            data=data, 
            cob_len=block_size,
            enable_log=enb_log)
        app.run(name)
            
if __name__ == '__main__':
    main()
