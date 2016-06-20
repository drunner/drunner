#ifndef __GENERATE_DBACKUP_H
#define __GENERATE_DBACKUP_H

#include "enums.h"
#include "utils.h"
#include "generate.h"

void generate_dbackup()
{
   std::string vdata = R"EOF(#!/bin/bash
set -o nounset
set -e

function die { echo "$1"; exit 1 ; }

# elementIn element array
# if elementIn "a string" "${array[@]}" ; then ...
function elementIn {
  local e
  for e in "${@:2}"; do [[ "$e" == "$1" ]] && return 0; done
  return 1
}

function showusage {
cat <<'EOF'
NAME
   dbackup

DESCRIPTION
   Provides backups across all installed dServices.

SYNOPSIS
   dbackup [COMMAND] [ARGS]...

COMMANDS
   dbackup configure BACKUPPATH
   dbackup include SERVICENAME
   dbackup exclude SERVICENAME
   dbackup info
   dbackup run
EOF

[ $# -eq 0 ] || (echo " "; echo "$1";)

exit 1
}

[ "$UID" -ne 0 ] || die "Please don't run dbackup as root."

[ "$#" -gt 0 ] || showusage 

CMD=$1
shift

validcmds=("configure" "include" "exclude" "run" "info")

if elementIn "$CMD" "${validcmds[@]}" ; then
   drunner "__dbackup_${CMD}" "$@"
else
   showusage "Unkown command ${CMD}"
fi

exit 0
}
)EOF";

   std::string target = utils::get_usersbindir() + "/dbackup";
   generate(target,S_755,vdata);
}


#endif
