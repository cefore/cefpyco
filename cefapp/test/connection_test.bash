#!/usr/bin/env bash
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

############################################################
# Utilities
############################################################

on_debug=true
w="$(tput cols)"
cout() { while read line; do printf "\033[3${1}m${line}\033[m\n"; done; }
p_err() { while read line; do echo "[ERROR] $line"; done | cout 1; exit 1; }
p_wrn() { while read line; do echo "[WARN] $line"; done | cout 1; }
p_inf() { while read line; do echo "[INFO] $line"; done | cout 2; }
p_dbg() { if $on_debug; then while read line; do echo "[DEBUG] $line"; done | cout 6; fi; }
p_prd() { while read line; do echo "[P] $line"; done | cout 3; }
p_cns() { while read line; do echo "__[C] $line"; done | cout 5; }
hr() { echo "$(python3 -c "print('='*$w)")"; }
fout() {
    nl=$(wc -l $1)
    nl="${nl%% *}"
    if (($nl > 100)); then
        echo "(Previews the first and last 50 lines out of $nl.)"
        head -n 50 $1
        for i in {1..5}; do echo "[...]"; done
        tail -n 50 $1
    else
        cat $1
    fi
}

############################################################
# Test
############################################################

setup() {
    export USER=root
    export tmpf="$(mktemp)"
    export cntf="$(mktemp)"
    export ceff="$(mktemp)"
    export logf="$(mktemp)"
    export errf="$(mktemp)"
    cefnetdstart >"$ceff" 2>&1
    sleep 0.5
}

teardown() {
    cefnetdstop || :
    rm -f "$tmpf" "$cntf" "$ceff" "$logf" "$errf"
    sleep 0.5
}

if [[ -x ./cefapp/cefappconsumer.py && -x ./cefapp/cefappproducer.py ]]; then
    :
else
    echo "Change the current directory to 'cefpyco'" | p_err
fi

assert() {
    {
        printf "assert: "
        for arg in "$@"; do
            printf "<$arg> "
        done
        echo
    } | p_dbg
    if "$@" > /dev/null; then
        echo "Succeeded on: '$*'" | p_dbg
    else
        echo "Failed on: '$*'" | p_err
    fi
}

############################################################
# Test cases
############################################################

case_001() {
    echo "Simple communication (arg/stdout mode)" | p_inf
    ./cefapp/cefappproducer.py ccnx:/test hello | p_prd &
    sleep 0.1
    ./cefapp/cefappconsumer.py ccnx:/test > "$tmpf" | p_cns
    echo "-----"
    cat "$tmpf"
    echo "-----"
    assert grep "hello" "$tmpf"
}

case_002() {
    echo "Simple communication (arg/file mode)" | p_inf
    content="hello"
    ./cefapp/cefappproducer.py ccnx:/test -i arg hello | p_prd &
    sleep 0.1
    ./cefapp/cefappconsumer.py ccnx:/test -f "$tmpf" | p_cns
    echo "-----"
    cat "$tmpf"
    echo "-----"
    assert grep "$content" "$tmpf"
}

case_003() {
    echo "Simple communication (file/file mode)" | p_inf
    content="hello"
    printf "$content" > "$cntf"
    ./cefapp/cefappproducer.py ccnx:/test -i file $cntf | p_prd &
    sleep 0.1
    ./cefapp/cefappconsumer.py ccnx:/test -o file -f "$tmpf" | p_cns
    echo "-----"
    cat "$tmpf"
    echo "-----"
    assert grep "$content" "$tmpf"
}

case_004() {
    echo "Transferring a large file" | p_inf
    content="$(seq -s . 30000)"
    printf "$content" > "$cntf"
    ./cefapp/cefappproducer.py ccnx:/test -i file $cntf | p_prd &
    sleep 0.1
    ./cefapp/cefappconsumer.py ccnx:/test -f "$tmpf" | p_cns
    echo "-----"
    cat "$tmpf"
    echo "-----"
    assert diff "$cntf" "$tmpf"
}

case_005() {
    echo "Receiving an Interest Return packet" | p_inf
    ./cefapp/cefappconsumer.py ccnx:/test > "$tmpf" 2>&1
    echo "-----"
    cat "$tmpf"
    echo "-----"
    assert grep "No Route" "$tmpf"
    false
}

############################################################
# Body
############################################################

for c in case_{001..005}; do
    setup
    ( set -ev; $c; exit 0; ) >$logf  2>$errf
    if [[ "$?" == 0 ]]; then
        echo "[SUCCESS] $c" | cout 2
    else
        {
            echo "[ERROR] $c"
            hr 
            fout $tmpf | cout 7
            hr 
            fout $logf | cout 7
            hr
            fout $errf
            hr 
            fout $ceff | cout 7
            cefstatus
            hr
            echo
        } | cout 1
    fi
    teardown
done
