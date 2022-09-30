 #!/bin/bash

if [[ $# != 1 ]]; then
  echo "Usage $0 <path-to-c-file>";
  exit 1;
fi

RVSDG_DIR=out/rvsdg
[ -d $RVSDG_DIR ] || mkdir -p $RVSDG_DIR

name=$(basename ${1%.*c})

./get_llvm_ir.sh $1

jlm-print --j2rx --file out/ll/$name.ll > out/rvsdg/$name.rvsdg
