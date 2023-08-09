#!/usr/bin/env bash

############################################################
### USAGE ##################################################
############################################################

USAGE=$(cat <<EOF
USAGE: $0 [-h|--help] [-i|--install]

DESCRIPTION:
    Build the project of cefpyco.
    Also install it with -i option.

OPTIONS:
    * -h|--help)    Show this message.
    * -i|--install) Enable install as well as build.
    * -c|--clean)   Clean the build files.

EOF
)

if [[ $* =~ -.*h ]]; then
    printf "\033[1;93m${USAGE}\033[m\n"
    exit
fi

############################################################
### CONFIGURATIONS AND UTILITIES ###########################
############################################################

set -e

_pD() { printf "\033[0;95m[  DEBUG] $*\033[m\n" 1>&2; }
_pI() { printf "\033[0;94m[   INFO] $*\033[m\n" 1>&2; }
_pW() { printf "\033[1;93m[WARNING] $*\033[m\n" 1>&2; }
_pS() { printf "\033[1;92m[SUCCESS] $*\033[m\n" 1>&2; }
_pE() {
    printf "\033[1;91m[ERROR] $*\033[m\n" 1>&2
    exit 1
}

############################################################
### BODY ###################################################
############################################################

###--- clean --------------------------------------------###

if [[ $* =~ -.*c ]]; then
    _pW "Clean mode."
    if [[ -f "Makefile" ]]; then 
        make clean
    fi
    rm -rf \
        CMakeCache.txt \
        ./{,src/,src/cefpyco_c/,src/cefpyco_c/cefparse/,src/cefpyco_c/test/}{Makefile,CMakeFiles,cmake_install.cmake}
    rm -rf \
        .pytest_cache \
        cefpyco.egg-info \
        src/cefpyco.egg-info \
        src/cefapp/__pycache__ \
        src/cefpyco/__pycache__ \
        build \
        dist \
        install_manifest.txt
    _pS "Clean process has completed."
    exit
fi

###--- debug --------------------------------------------###

if [[ $* =~ -.*g ]]; then
    _tmp_dir=$(mktemp -d /tmp/tmp.cefpyco.XXXXXXXX)
    cp -r . ${_tmp_dir}
    cd ${_tmp_dir}
    _pD "Build in the debug directory '${_tmp_dir}'."
    if [[ $* =~ -.*G ]]; then
        _pD "The debug directory will be removed soon."
        trap "rm -rf /tmp/tmp.cefpyco.????????" EXIT
    fi
fi

###--- build --------------------------------------------###

GET_VERSION_PYTHON_CODE=$(cat <<EOF
import toml
with open("pyproject.toml") as f:
    t = toml.load(f)
print(t["project"]["version"])
EOF
)

version=$(python3 -c "${GET_VERSION_PYTHON_CODE:?}")
_pI "Start to build cefpyco-${version:?}."
cmake -Dcefpyco_ver="$version" .
make
_pS "Build process has completed."

###--- install ------------------------------------------###

if [[ $* =~ -.*i ]]; then
    _pI "Start to install cefpyco-${version:?}."
    if which sudo; then
        if sudo -l | grep -q "ALL"; then
            make install
        else
            _pW "Requires sudo privileges."
            sudo make install
        fi
    else
        make install
    fi
    _pS "Install process has completed."
fi
