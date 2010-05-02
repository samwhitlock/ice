#!/usr/bin/env bash

success_count=0
failure_count=0

for test in ${@} ; do \
    echo -e "\e[1;33m>>\e[0m Running test suite '$test'..."

    ${test}
    exit_status=${?}

    if [[ ${exit_status} == 0 ]] ; then
        echo -e "\e[1;32m>>\e[0m Success!"
        (( success_count++ ))
    else
        echo -e "\e[1;31m>>\e[0m Failure, test exited with status ${exit_status}"
        (( failure_count++ ))
    fi
done

echo -e "\e[1;34m>>\e[0m Results: ${success_count} successes, ${failure_count} failures"

[[ ${failure_count} == 0 ]] && exit 0 || exit 1

