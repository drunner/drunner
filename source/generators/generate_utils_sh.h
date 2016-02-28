#ifndef __CREATEUTILS_SH_H
#define __CREATEUTILS_SH_H

#include "enums.h"
#include "params.h"
#include "utils.h"
#include "generate.h"

void generate_utils_sh(const std::string & supportpath, const params & p)
{
std::string vdata=R"EOF(#!/bin/bash

# --- some useful utility functions
# --- generally should be okay if used with 'set -e'.

# Formatting for comamnds - standardised.
[ -v ECODE ] || readonly ECODE=$(printf "\e")
[ -v CODE_S ] || readonly CODE_S="$ECODE[32m"
[ -v CODE_E ] || readonly CODE_E="$ECODE[0m"

#------------------------------------------------------------------------------------
# die MSG [EXITCODE] - show the message (red) and exit exitcode.

function die {
   echo " ">&2 ; echo -e "\e[31m\e[1m${1}\e[0m">&2  ; echo " ">&2
   EXITCODE=${2:-1}
   exit "$EXITCODE"
}

#------------------------------------------------------------------------------------
# See if a container is exists. Return value of docker inspect is correct (0 if running)
# https://gist.github.com/ekristen/11254304

function container_exists {
   [ $# -eq 1 ] || die "container_exists requires containername parameter."
   # exit code of docker inspect says if it's running. Don't let the line fail though (in case we have set -e)
   docker inspect --format="{{ .State.Running }}" "$1" >/dev/null 2>&1 || return 1
}

#------------------------------------------------------------------------------------
# See if a container is running

function container_running {
   [ $# -eq 1 ] || die "container_running requires containername parameter."
   container_exists "$1" && [ $(docker inspect --format="{{ .State.Running }}" "$1" 2>/dev/null) = "true" ]
}

#------------------------------------------------------------------------------------
# See if a container is paused

function container_paused {
   [ $# -eq 1 ] || die "container_paused requires containername parameter."
   container_exists "$1" && [ $(docker inspect --format="{{ .State.Paused }}" "$1" 2>/dev/null) = "true" ]
}

#------------------------------------------------------------------------------------
# Import directory into container. utils_import source-localpath dest-containerpath

function utils_import {
   [ "$#" -eq 2 ] || die "utils_import -- requires two arguments (the path to be imported and the container's destination path)."
   [ -d "$1" ] || die "utils_import -- source path does not exist: $1"

   dockerrun bash -c "rm -r $2/*"
   tar cf - -C "$1" . | docker run -i --name="${SERVICENAME}-importfn" "${DOCKEROPTS[@]}" "${IMAGENAME}" tar -xv -C "$2"
   RVAL=$?
   docker rm "${SERVICENAME}-importfn" >/dev/null
   [ $RVAL -eq 0 ] || die "utils_import failed to transfer the files."
}

#------------------------------------------------------------------------------------
# Export directory from container. utils_export source-containerpath dest-localpath

function utils_export {
   [ "$#" -eq 2 ] || die "utils_export -- requires two arguments (the container's source path and the path to be exported to)."
   [ -d "$2" ] || die "utils_export -- destination path does not exist."

   docker run -i --name="${SERVICENAME}-exportfn" "${DOCKEROPTS[@]}" "${IMAGENAME}" tar cf - -C "$1" . | tar -xv -C "$2"
   RVAL=$?
   docker rm "${SERVICENAME}-exportfn" >/dev/null
   [ $RVAL -eq 0 ] || die "utils_export failed to transfer the files."
}

)EOF";

   generate(supportpath+"/utils.sh",p,vdata);
}

#endif
