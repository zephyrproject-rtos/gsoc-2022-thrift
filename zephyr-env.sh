if [ ! -f ${PWD}/zephyr-env.sh ]; then
    echo "Please source this script from the 'thrift-for-zephyr' directory"
fi

if [ -f ~/.zephyrrc ]; then
    . ~/.zephyrrc;
fi

export ZEPHYR_BASE="$(cd "${PWD}"/../zephyr; pwd)"
export PATH=${ZEPHYR_BASE}/scripts:${PATH}
