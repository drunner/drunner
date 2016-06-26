#ifndef __GENERATE_PLUGIN_SCRIPT_H
#define __GENERATE_PLUGIN_SCRIPT_H


#include "enums.h"
#include "utils.h"
#include "generate.h"

void generate_plugin_script (std::string pluginname)
{
   std::string vdata = R"EOF(#!/bin/bash
#
# ----------------------------------------
#
# Plugin launcher for __PLUGINNAME__
#
# ----------------------------------------

set -o nounset
set -e

PLUGINNAME="__PLUGINNAME__"

function die { echo "$1"; exit 1 ; }

[ "$UID" -ne 0 ] || die "Please don't run as root."

CMD="help"
[ "$#" -eq 0 ] || { CMD=$1 ; shift ; }

drunner "__plugin__${PLUGINNAME}" "${CMD}" "$@"

)EOF";

   vdata = utils::replacestring(vdata, "__PLUGINNAME__", pluginname);
   std::string target = utils::get_usersbindir() + "/"+pluginname;
   generate(target, S_755, vdata);
}

#endif

