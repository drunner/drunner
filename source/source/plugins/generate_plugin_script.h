#ifndef __GENERATE_PLUGIN_SCRIPT_H
#define __GENERATE_PLUGIN_SCRIPT_H


#include "enums.h"
#include "utils.h"
#include "generate.h"
#include "drunner_paths.h"


#ifdef _WIN32


void generate_plugin_script(std::string pluginname)
{
   std::string vdata = R"EOF(
@echo off
__DRUNNER__ __plugin____PLUGINNAME__ %*
)EOF";

   vdata = utils::replacestring(vdata, "__PLUGINNAME__", pluginname);
   vdata = utils::replacestring(vdata, "__DRUNNER__", drunnerPaths::getPath_Exe_Target().toString());

   Poco::Path target = drunnerPaths::getPath_Bin().setFileName(pluginname + ".bat");
   generate(target, S_755, vdata);
}


#else


void generate_plugin_script (std::string pluginname)
{
   std::string vdata = R"EOF(#!/bin/sh
#
# -----------------------------------------
#
# Plugin launcher for __PLUGINNAME__
#
# -----------------------------------------

__DRUNNER__ "__plugin____PLUGINNAME__" "$@"

)EOF";

   vdata = utils::replacestring(vdata, "__PLUGINNAME__", pluginname);
   vdata = utils::replacestring(vdata, "__DRUNNER__", drunnerPaths::getPath_Exe_Target().toString());

   Poco::Path target = drunnerPaths::getPath_Bin().setFileName(pluginname);
   generate(target, S_755, vdata);
}

#endif

#endif

