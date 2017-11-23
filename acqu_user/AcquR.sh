#!/bin/bash

PROG=$(readlink -f "${0}")
ACQU=$(dirname "${PROG}")

WD=$PWD
FILE=$(readlink -f "${1}")

cd $ACQU

ARFILE=$(mktemp --tmpdir="./data" AR-XXXXXXX.dat)

cat ./data/AR_template.dat | sed "s#%%FILE%%#${FILE}#g" | sed "s#%%OUTDIR%%#${WD}#g" > "${ARFILE}"

echo "Temporary AR file: ${ARFILE}"

../build/bin/AcquRoot --batch ${ARFILE}

rm "${ARFILE}"

cd "${WD}"

if [ $# -eq 2 ]; then
	echo Moving Acqu_${FILE} to "${2}"...
	basen=$(basename ${FILE})
	mv Acqu_${basen} "${2}"
fi

