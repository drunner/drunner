#ifndef __GENERATE_PLUGIN_SCRIPT_H
#define __GENERATE_PLUGIN_SCRIPT_H


#include "enums.h"
#include "utils.h"
#include "generate.h"
#include "drunner_paths.h"

void generate_plugin_script (std::string pluginname)
{
   std::string vdata = R"EOF(#!/bin/bash
#
# -----------------------------------------
#
# Plugin launcher for __PLUGINNAME__
#
# This script must be in the same directory
# as the drunner link ($HOME/bin)
#
# -----------------------------------------

set -o nounset
set -e

MYDIR=$( dirname "$(readlink -f "$0")" )
PLUGINNAME="__PLUGINNAME__"

function die { echo "$1"; exit 1 ; }

[ "$UID" -ne 0 ] || die "Please don't run as root."

CMD="help"
[ "$#" -eq 0 ] || { CMD=$1 ; shift ; }

"${MYDIR}/drunner" "__plugin__${PLUGINNAME}" "${CMD}" "$@"

)EOF";

   vdata = utils::replacestring(vdata, "__PLUGINNAME__", pluginname);
   Poco::Path target = drunnerPaths::getPath_Bin().setFileName(pluginname);
   generate(target, S_755, vdata);
}

#endif

