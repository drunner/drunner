#include "Poco/String.h"
#include "Poco/StringTokenizer.h"
#include "Poco/Process.h"

#include "service_lua.h"
#include "dassert.h"
#include "utils_docker.h"
#include "proxy.h"

// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

// lua C functions - defined in service_lua_cfuncs
extern "C" int l_addconfig(lua_State *L);
extern "C" int l_getconfig(lua_State *L);
extern "C" int l_setconfig(lua_State *L);

extern "C" int l_drun(lua_State *L);
extern "C" int l_drunoutput(lua_State *L);
extern "C" int l_drunoutputexit(lua_State *L);
extern "C" int l_dsub(lua_State *L);
extern "C" int l_dsplit(lua_State *L);

extern "C" int l_getdrundir(lua_State *L);
extern "C" int l_setdrundir(lua_State *L);
extern "C" int l_getpwd(lua_State *L);
extern "C" int l_isdockerrunning(lua_State *L);

extern "C" int l_docker(lua_State *L);
extern "C" int l_dockerti(lua_State *L);

extern "C" int l_dockerstop(lua_State *L);
extern "C" int l_dockerwait(lua_State *L);
extern "C" int l_dockerpull(lua_State *L);
extern "C" int l_dockercreatevolume(lua_State *L);
extern "C" int l_dockerdeletevolume(lua_State *L);
extern "C" int l_dockerbackup(lua_State *L);
extern "C" int l_dockerrestore(lua_State *L);

extern "C" int l_proxyenable(lua_State *L);
extern "C" int l_proxydisable(lua_State *L);

extern "C" int l_die(lua_State *L);
extern "C" int l_dieif(lua_State *L);
extern "C" int l_dieunless(lua_State *L);


#define REGISTERLUAC(cfunc,luaname) lua_pushcfunction(L, cfunc); lua_setglobal(L, luaname);

namespace servicelua
{
   // -----------------------------------------------------------------------------------------------------------------------

   void _register_lua_cfuncs(lua_State *L)
   { // define all our C functions that we want available from service.lua

      REGISTERLUAC(l_addconfig, "addconfig")
      REGISTERLUAC(l_getconfig, "getconfig")
      REGISTERLUAC(l_setconfig, "setconfig")

      REGISTERLUAC(l_drun, "drun")

      REGISTERLUAC(l_dsub, "dsub")
      REGISTERLUAC(l_dsplit, "dsplit")

      REGISTERLUAC(l_getdrundir,"getdrundir")
      REGISTERLUAC(l_setdrundir, "setdrundir")
      REGISTERLUAC(l_getpwd, "getpwd")

      REGISTERLUAC(l_docker, "docker")
      REGISTERLUAC(l_dockerti, "dockerti")

      REGISTERLUAC(l_dockerstop, "dockerstop")
      REGISTERLUAC(l_isdockerrunning, "isdockerrunning")
      REGISTERLUAC(l_dockerwait, "dockerwait")
      REGISTERLUAC(l_dockerpull, "dockerpull")
      REGISTERLUAC(l_dockercreatevolume, "dockercreatevolume")
      REGISTERLUAC(l_dockerdeletevolume, "dockerdeletevolume")
      REGISTERLUAC(l_dockerbackup, "dockerbackup")
      REGISTERLUAC(l_dockerrestore, "dockerrestore")

      REGISTERLUAC(l_proxyenable, "proxyenable")
      REGISTERLUAC(l_proxydisable, "proxydisable")

      REGISTERLUAC(l_die, "die")
      REGISTERLUAC(l_dieif, "dieif")
      REGISTERLUAC(l_dieunless, "dieunless")
   }

   // -----------------------------------------------------------------------------------------------------------------------


   luafile * get_luafile(lua_State *L)
   {
      int stacksize = lua_gettop(L);
      lua_getglobal(L, "luafile");
      drunner_assert(lua_islightuserdata(L, -1), "Is not lightuserdata.");
      void * luafilevoid = lua_touserdata(L, -1); 
      drunner_assert(luafilevoid != NULL, "`luafile' expected");
      lua_pop(L, 1); // pop top element
      drunner_assert(stacksize == lua_gettop(L), "Stack unbalanced : "+std::to_string(stacksize)+" != "+std::to_string(lua_gettop(L)));
      return (luafile *)luafilevoid;
   }


   // -----------------------------------------------------------------------------------------------------------------------

   int _luasuccess(lua_State *L)
   {
      lua_pushboolean(L, true);
      return 1; // one argument to return.
   }

   int _luafail(lua_State *L, std::string failmessage)
   {
      logmsg(kLWARN, failmessage);
      lua_pushboolean(L, false);
      return 1;
   }

   int _luacresult(lua_State *L, cResult r)
   {
      if (r.success())
         return _luasuccess(L);
      else
         return _luafail(L, r.what());
   }

   // -----------------------------------------------------------------------------------------------------------------------

   // -----------------------------------------------------------------------------------------------------------------------

   // just sanity check the pass we've already done.
   extern "C" int l_addconfig(lua_State *L)
   {
      if (lua_gettop(L) != 3)
         return _luafail(L, "Expected three arguments for addconfig.");

      // name(n), defaultval(dflt), description(desc), type(t), required(rqd), usersettable(user) {}
      drunner_assert(lua_isstring(L, 1), "The name must be a string.");
      drunner_assert(lua_isstring(L, 2), "The default value must be a string.");
      drunner_assert(lua_isstring(L, 3), "The description must be a string.");

      envDef def(lua_tostring(L, 1), lua_tostring(L, 2), lua_tostring(L, 3), ENV_PERSISTS | ENV_USERSETTABLE);
      const std::vector<envDef> & config = servicelua::get_luafile(L)->getServiceVars().getEnvDefs();
      bool found = false;
      for (auto & x : config)
         if (0 == Poco::icompare(x.name, def.name))
         {
            found = true;

            if (0 != Poco::icompare(x.defaultval,def.defaultval))
               logmsg(kLWARN, "Inconsistency of default value between quick read and lua execution for config " + def.defaultval);

            if (0 != Poco::icompare(x.description, def.description))
               logmsg(kLWARN, "Inconsistency of description between quick read and lua execution for config " + def.defaultval);

            break;
         }

      if (!found)
         logmsg(kLWARN, "Inconsistency of name between quick read and lua execution for config " + def.defaultval);

      return _luasuccess(L);
   }

   // -----------------------------------------------------------------------------------------------------------------------
   
   // convert args (either as separate args or a lua table) to a vec of strings.
   std::vector<std::string> args2vec(lua_State *L)
   {
      std::vector<std::string> vs;

      if (lua_gettop(L) == 0)
         return {};

      if (lua_isstring(L, 1) == 1)
      {
         for (int i = 1; i <= lua_gettop(L); ++i)
         {
            drunner_assert(lua_isstring(L, i), "arg must be a string");
            vs.push_back(lua_tostring(L, i));
         }
      }
      else if (lua_istable(L, 1) == 1)
      {
         int t = 1;
         lua_pushnil(L); // we want the first key.
         while (lua_next(L, t) != 0)
         {
            drunner_assert(lua_isnumber(L, -2), "Table keys aren't numbers");
            drunner_assert(lua_isstring(L, -1), "Table value isn't a string");

            //int i = (int)lua_tonumber(L, -2); // key
            std::string s = lua_tostring(L, -1); // value
                                                 //logmsg(kLDEBUG, std::to_string(i)+" -> [" + s + "]");

            vs.push_back(s);
            lua_pop(L, 1); // remove value, keep key for next iteration.
         }
      }
      else
         logmsg(kLERROR, "Unrecognised argument type. Unable to parse arguments.");
      
      return vs;
   }

   // -----------------------------------------------------------------------------------------------------------------------

   // returns result, output.
   int _drun(lua_State * L, CommandLine operation)
   {
      luafile * lf = get_luafile(L);
      std::string out;
      int r = utils::runcommand_stream(
         operation,
         kORaw,
         lf->getdRunDir(),
         lf->getServiceVars().getAll(),
         &out);

      lua_pushboolean(L, r==0);
      Poco::trimInPlace(out);
      lua_pushstring(L, out.c_str());
      return 2;
   }

   extern "C" int l_drun(lua_State *L)
   {
      CommandLine operation;
      operation.setfromvector(args2vec(L));

      return _drun(L, operation);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_docker(lua_State *L)
   {
      CommandLine operation;
      std::vector<std::string> args(args2vec(L));
      args.insert(args.begin(), "docker");
      operation.setfromvector(args);
      return _drun(L, operation);
   }


   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dockerti(lua_State *L)
   {
      if (lua_gettop(L) < 1)
         return _luafail(L, "Expected at least one argument: dockerti( arg1, arg2, ... )");

      CommandLine operation;
      std::vector<std::string> args(args2vec(L));
      args.insert(args.begin(), { "docker","-ti","--rm" });
      operation.setfromvector(args);

      int rval = -1;
      try {
         luafile * lf = get_luafile(L);
         Poco::ProcessHandle ph = Poco::Process::launch(operation.command, operation.args,
            lf->getdRunDir().toString(), NULL,NULL,NULL, lf->getServiceVars().getAll());

         rval = ph.wait();
      }
      catch (Poco::SystemException & se)
      {
         logmsg(kLWARN,se.displayText());
      }
      return (rval == 0) ? _luasuccess(L) : _luafail(L, "Failed to launch docker command");
   }


   // -----------------------------------------------------------------------------------------------------------------------
      
   extern "C" int l_isdockerrunning(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return _luafail(L, "Expected exactly one argument (the container name to check) for drunning.");
      drunner_assert(lua_isstring(L, 1), "container name must be a string.");
      std::string containerraw = lua_tostring(L, 1);
      //luafile *lf = get_luafile(L);

      bool running = utils_docker::dockerContainerRunning(containerraw);
      lua_pushboolean(L,running);
      return 1;
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dockerwait(lua_State *L)
   {
      // dockerwait( containername, port, timeout=30s)
      // Waits until port is up in the container. Times out after 30 seconds by default
      int nargs = lua_gettop(L);
      if (nargs < 2 || nargs > 3)
         return _luafail(L, "Incorrect number of arguments. Syntax:   dockerwait( containername, port, [timeout] )");
      std::string containername = lua_tostring(L, 1);
      int port = (int)lua_tointeger(L, 2);
      int timeout = (nargs == 3 ? (int)lua_tointeger(L, 3) : 30);

      if (utils_docker::dockerContainerWait(containername, port, timeout))
         return _luasuccess(L);

      return _luafail(L, "Container " + containername + " never came up on port " + port);
   }


   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dockerpull(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return _luafail(L, "Expected exactly one argument (the image name to pull) for dockerpull.");
      drunner_assert(lua_isstring(L, 1), "image name must be a string.");
      std::string imagename = lua_tostring(L, 1);
      //luafile *lf = get_luafile(L);

      cResult r = utils_docker::pullImage(imagename);

      return _luacresult(L, r);
   }
   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dockerstop(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return _luafail(L, "Expected exactly one argument (the container name to stop) for dockerstop.");
      drunner_assert(lua_isstring(L, 1), "container name must be a string.");
      std::string containerraw = lua_tostring(L, 1);

      luafile *lf = get_luafile(L);
      std::string subcontainer = lf->getServiceVars().substitute(containerraw);

      if (utils_docker::dockerContainerRunning(subcontainer))
      {
         logmsg(kLINFO, "Stopping container "+subcontainer);
         utils_docker::stopContainer(subcontainer);
      }
      if (utils_docker::dockerContainerExists(subcontainer))
      {
         logmsg(kLINFO, "Removing container "+ subcontainer);
         utils_docker::removeContainer(subcontainer);
      }
      else
         logmsg(kLDEBUG, subcontainer + " is not running.");
      return _luasuccess(L);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dockercreatevolume(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return _luafail(L, "Expected exactly one argument (the volume to create) for dockercreatevolume.");
      drunner_assert(lua_isstring(L, 1), "volume name must be a string.");
      std::string vol = lua_tostring(L, 1);
      cResult r = utils_docker::createDockerVolume(vol);

      return _luacresult(L, r);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dockerdeletevolume(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return _luafail(L, "Expected exactly one argument (the volume to delete) for dockerdeletevolume.");
      drunner_assert(lua_isstring(L, 1), "volume name must be a string.");
      std::string vol = lua_tostring(L, 1);
      cResult r = utils_docker::deleteDockerVolume(vol);

      return _luacresult(L, r);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dockerbackup(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return _luafail(L, "Expected exactly one argument (the volume to backup) for dockerbackup.");
      drunner_assert(lua_isstring(L, 1), "volume name must be a string.");
      std::string vol = lua_tostring(L, 1);

      luafile * lf = get_luafile(L);
      drunner_assert(lf->getServiceVars().getTempBackupFolder().length() > 0, "Temp Backup folder not set! Only call dockerbackup from your Lua backup() function.");
      Poco::Path folder(lf->getServiceVars().getTempBackupFolder());

      cResult r = utils_docker::backupDockerVolume(vol,folder,lf->getServiceName());

      return _luacresult(L, r);
   }
   
   // -----------------------------------------------------------------------------------------------------------------------
   
   extern "C" int l_dockerrestore(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return _luafail(L, "Expected exactly one argument (the volume to restore) for dockerrestore.");
      drunner_assert(lua_isstring(L, 1), "volume name must be a string.");
      std::string vol = lua_tostring(L, 1);

      luafile * lf = get_luafile(L);
      drunner_assert(lf->getServiceVars().getTempBackupFolder().length() > 0, "Temp Backup folder not set! Coding error.");
      Poco::Path folder(lf->getServiceVars().getTempBackupFolder());

      cResult r = utils_docker::restoreDockerVolume(vol, folder, lf->getServiceName());

      return _luacresult(L, r);
   }
   
   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dsub(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return _luafail(L, "Expected exactly one argument (the string to substitute) for dsub.");

      drunner_assert(lua_isstring(L, 1), "String expected as argument.");
      luafile * lf = get_luafile(L);
      std::string s = lf->getServiceVars().substitute(lua_tostring(L, 1));
      lua_pushstring(L, s.c_str());
      return 1; // one argument to return.
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_getconfig(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return _luafail(L, "Expected exactly one argument (the variable name) for dconfig_get.");

      drunner_assert(lua_isstring(L, 1), "String expected as argument.");
      std::string s = lua_tostring(L, 1);
      luafile *lf = get_luafile(L);
      std::string v = lf->getServiceVars().getVal(s);
      lua_pushstring(L, v.c_str());
      return 1; // one arg to return.
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_setconfig(lua_State *L)
   {
      if (lua_gettop(L) != 2)
         return _luafail(L, "Expected exactly two arguments (the variable name and value) for dconfig_set.");
      luafile *lf = get_luafile(L);

      drunner_assert(lua_isstring(L, 1), "String expected as argument.");
      drunner_assert(lua_isstring(L, 2), "String expected as argument.");
      std::string s = lua_tostring(L, 1);
      std::string v = lua_tostring(L, 2);

      cResult r = lf->getServiceVars().setVal(s, v);
      r += lf->getServiceVars().savevariables();

      if (r == kRError)
         logmsg(kLWARN, "Failed to set " + s + " to " + v+":\n "+r.what());

      return _luacresult(L, 1);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dsplit(lua_State *L)
   {
      if (lua_gettop(L)!=1)
         return _luafail(L, "Expected exactly one argument (the string to split) for dsplit.");

      drunner_assert(lua_isstring(L, 1), "String expected as argument.");
      std::string s = lua_tostring(L, 1);

      // tokenize string, recognising quoted substrings and escaped characters.
      std::vector<std::string> tok;
      std::string elem;
      for (size_t i = 0; i<s.length(); ++i) {
         switch (s[i])
         {
         case ' ':
            tok.push_back(elem);
            elem.clear();
            break;

         case '\\':
            ++i;
            drunner_assert(i < s.length(), "Incomplete escape sequence.");
            elem += s[i];
            break;

         case '"':
            ++i;
            while (i < s.length() && s[i] != '"') { elem += s[i]; ++i; }
            break;

         default:
            elem += s[i];
         }
      }
      if (elem.length() > 0)
         tok.push_back(elem);

      // output the split string into a Lua table.
      lua_createtable(L, tok.size(),0);
      int top = lua_gettop(L);

      for (unsigned int i = 0; i < tok.size(); ++i)
      {
         lua_pushnumber(L, i+1);
         lua_pushstring(L, tok[i].c_str());
         lua_settable(L, top);
      }

      return 1;
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_getdrundir(lua_State *L)
   {
      luafile *lf = get_luafile(L);
      lua_pushstring(L, lf->getdRunDir().toString().c_str());
      return 1;
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_setdrundir(lua_State *L)
   {
      std::string s;

      if (lua_gettop(L) > 1)
         return _luafail(L, "Expected exactly one argument (the new directory) for setpwd.");

      if (lua_gettop(L) == 1)
      {
         drunner_assert(lua_isstring(L, 1), "String expected as argument.");
         s = lua_tostring(L, 1);
      }

      luafile *lf = get_luafile(L);
      lf->setdRunDir(s);

      return _luasuccess(L);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_getpwd(lua_State *L)
   {
      Poco::Path cfp = Poco::Path::current();
      lua_pushstring(L, cfp.toString().c_str());
      return 1;
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_proxyenable(lua_State *L)
   {
      if (lua_gettop(L) < 6 || lua_gettop(L) > 7)
      {
         logmsg(kLWARN, "Expected 5 to 6 arguments: proxyenable( HOSTNAME, CONTAINER, PORT, NETWORK, EMAIL, MODE, [TIMEOUTS] )");
      }

      luafile *lf = get_luafile(L);
      std::string servicename = lf->getServiceName();

      drunner_assert(lua_isstring(L, 1), "proxyenable: String expected as 1st argument.");
      drunner_assert(lua_isstring(L, 2), "proxyenable: String expected as 2nd argument.");
      drunner_assert(lua_isnumber(L, 3), "proxyenable: Number expected as 3rd argument.");
      drunner_assert(lua_isstring(L, 4), "proxyenable: String expected as 4th argument.");
      drunner_assert(lua_isstring(L, 5), "proxyenable: String expected as 5th argument.");
      drunner_assert(lua_isstring(L, 6), "proxyenable: String expected as 6th argument.");
      drunner_assert(lua_gettop(L)==6 || lua_isboolean(L, 7), "proxyenable: Boolean expected as 7th optional argument.");

      bool timeouts = true;
      if (lua_gettop(L) == 7)
         timeouts = (lua_toboolean(L, 7)==1);
      if (!timeouts)
         logmsg(kLDEBUG, "Timeouts disabled for " + servicename);

      proxy p;
      cResult r = p.proxyenable(
         proxydatum(
            servicename,
            lua_tostring(L, 1),
            lua_tostring(L, 2),
            std::to_string(lua_tointeger(L, 3)),
            lua_tostring(L, 4),
            lua_tostring(L, 5),
            lua_tostring(L, 6),
            timeouts
         )
      );

      if (r == kRError)
         logmsg(kLWARN, "Failed to enable proxy: " + r.what());

      return _luacresult(L, r);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_proxydisable(lua_State *L)
   {
      if (lua_gettop(L) != 0)
         logmsg(kLWARN,"No arguments expected for proxydisable.");

      luafile *lf = get_luafile(L);
      std::string servicename = lf->getServiceName();

      proxy p;
      cResult r = p.proxydisable(servicename);

      if (r == kRError)
         logmsg(kLWARN, "Failed to disable proxy for "+servicename+":\n " + r.what());

      return _luacresult(L, r);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   int _die(std::string msg)
   {
      logmsg(kLWARN, "Lua die command executed:");
      fatal(msg);

      return 0; // never gets called.
   }

   extern "C" int l_die(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         fatal("No message passed to die.");
      return _die(lua_tostring(L, 1));
   }
   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dieif(lua_State *L)
   {
      if (lua_gettop(L) != 2)
         fatal("No message passed to dieif.");
      if (lua_toboolean(L, 1) == 1)
         return _die(lua_tostring(L, 2));
      
      return 0;
   }


   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dieunless(lua_State *L)
   {
      if (lua_gettop(L) != 2)
         fatal("No message passed to dieif.");
      if (lua_toboolean(L, 1) == 0)
         return _die(lua_tostring(L, 2));

      return 0;
   }

   // -----------------------------------------------------------------------------------------------------------------------


}
