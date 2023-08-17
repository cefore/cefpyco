#!/usr/bin/env bash

############################################################
### USAGE ##################################################
############################################################

USAGE=$(
    cat <<'EOF'
USAGE: $0 [-h|--help] [-i|--install] [-c|--clean] [-m|--mac]

DESCRIPTION:
    Build the project of cefpyco.
    Also install it with -i option.

OPTIONS:
    * -h|--help     Show this message.
    * -i|--install  Enable install as well as build.
    * -c|--clean    Clean the build files.
    * -x|--osx      OS X mode for Mac PC (assuming pyenv).

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

if [[ $* =~ -.*c ]]; then
    _ON_CLEAN_MODE=true
else
    _ON_CLEAN_MODE=false
fi

if [[ $* =~ -.*g ]]; then
    _ON_DEBUG_MODE=true
else
    _ON_DEBUG_MODE=false
fi

if [[ $* =~ -.*i ]]; then
    _ON_INSTALL_MODE=true
else
    _ON_INSTALL_MODE=false
fi

if [[ $* =~ -.*m ]] || [[ $(uname) =~ Darwin ]]; then
    _pW "Mac mode."
    _ON_MAC_MODE=true
else
    _ON_MAC_MODE=false
fi

GET_VERSION_PYTHON_CODE_WITH_TOML=$(
    cat <<EOF
import toml
with open("pyproject.toml") as f:
    t = toml.load(f)
print(t["project"]["version"])
EOF
)

GET_VERSION_PYTHON_CODE_WITH_CFG=$(
    cat <<EOF
from configparser import ConfigParser
cfg = ConfigParser()
cfg.read('setup.cfg')
print(cfg.get('metadata', 'version'))
EOF
)

############################################################
### BODY ###################################################
############################################################

###--- clean --------------------------------------------###

if ${_ON_CLEAN_MODE:?}; then
    _pW "Clean mode."
    if [[ -f "Makefile" ]]; then
        make clean
    fi
    _rmdir_list=",src/,test/"
    _rmdir_list="${_rmdir_list},src/cefpyco/,src/cefpyco_wrap/"
    _rmdir_list="${_rmdir_list},src/cefpyco_c/,src/cefpyco_c/cefparse/"
    _rmfile_list="CMakeCache.txt,Makefile,CMakeFiles,cmake_install.cmake"
    _rmfile_list="${_rmfile_list},CTestTestfile.cmake,__pycache__"
    for _f in $(eval echo {${_rmdir_list:?}}{${_rmfile_list:?}}); do
        rm -rf ./${_f:?}
    done
    rm -rf ./src/cefpyco/{libcefpyco,libcefpyco_wrap}{.so,.dylib}
    rm -rf \
        .pytest_cache \
        cefpyco.egg-info \
        Testing \
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

if ${_ON_DEBUG_MODE:?}; then
    _tmp_dir=$(mktemp -d /tmp/tmp.cefpyco.XXXXXXXX)
    cp -r . ${_tmp_dir}
    cd ${_tmp_dir}
    _pD "Build in the debug directory '${_tmp_dir}'."
    if [[ $* =~ -.*G ]]; then
        _pD "The debug directory will be removed soon."
        trap "rm -rf /tmp/tmp.cefpyco.????????" EXIT
    fi
fi

###--- check --------------------------------------------###

_PY_GET_VAR() {
    python -c "$(
        cat <<-EOF
	from sysconfig import get_config_var as _g
	print(_g('${1:?}'))
	EOF
    )"
}

if ${_ON_MAC_MODE:?}; then
    _enabled_framework="$(_PY_GET_VAR 'PYTHONFRAMEWORK')"
    if [[ "${_enabled_framework}" == "None" ]]; then
        _pE "
        Python framework is not enabled (not installed for develpment).
        If using pyenv, install with required options as following command:
        > env PYTHON_CONFIGURE_OPTS='--enable-framework' pyenv install 3.x.x
        "
    else
        _pI "Python framework is enables."
        _base_dir="$(_PY_GET_VAR 'base')"
        _pD "Python_ROOT_DIR: ${_base_dir}"
    fi
fi

###--- build --------------------------------------------###

version=$(python3 -c "${GET_VERSION_PYTHON_CODE_WITH_CFG:?}")
_pI "Start to build cefpyco-${version:?}."
if ${_ON_MAC_MODE:?}; then
    _cmake_opt="${_cmake_opt} -DPython_ROOT_DIR=${_base_dir}"
    _cmake_opt="${_cmake_opt} -DOPENSSL_ROOT_DIR=$(brew --prefix openssl)"
    _pW "$_cmake_opt"
fi
if ${_ON_DEBUG_MODE:?}; then
    cmake -DCMAKE_BUILD_TYPE=Debug ${_cmake_opt} .
    make CEFPYCO_DEBUG=
else
    cmake ${_cmake_opt} .
    make
fi
_pS "Build process has completed."

###--- install ------------------------------------------###

if ${_ON_INSTALL_MODE:?}; then
    _pI "Start to install cefpyco-${version:?}."
    make install

    # if ${_ON_MAC_MODE:?}; then
    #     py=$(brew --prefix python)/bin/python3.11
    #     $py -m build
    #     $py -m pip install dist/cefpyco-${version:?}.tar.gz
    # else
    #     make install
    # fi

    # if which sudo; then
    #     if sudo -l | grep -q "ALL"; then
    #         make install
    #     else
    #         _pW "Requires sudo privileges."
    #         sudo make install
    #     fi
    # else
    #     make install
    # fi
    _pS "Install process has completed."
fi
