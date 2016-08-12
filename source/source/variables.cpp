#include <Poco/String.h>
#include <Poco/Environment.h>
#include <fstream>
#include <cereal/archives/json.hpp>

#include "variables.h"
#include "utils.h"
#include "globallogger.h"
#include "dassert.h"


//variables::variables(const variables & other)
//{
//   const tKeyVals & otherkvs(other.getAll());
//   mVariables.insert(otherkvs.begin(), otherkvs.end());
//}
variables::variables(const variables & other1, const variables & other2)
{
   mVariables = other1.getAll();
   for (const auto & x : other2.getAll())
      mVariables[x.first] = x.second;
}

bool variables::hasKey(std::string key) const
{
   for (const auto & x : mVariables)
      if (0==Poco::icompare(key, x.first))
         return true;
   return false;
}

std::string variables::getVal(std::string key) const
{
   for (const auto & x : mVariables)
      if (0==Poco::icompare(key, x.first))
         return x.second;
   return "";
}

bool variables::getBool(std::string key) const
{
   std::string s = getVal(key);
   if (s.length() == 0) return false;
   return (tolower(s[0]) == 'y' || tolower(s[0]) == 't');
}

void variables::setVal(std::string key, std::string val) 
{
   mVariables[key] = val;
}

void variables::delKey(std::string key)
{
   mVariables.erase(key);
}


std::string variables::substitute(std::string s) const
{
   std::string os(s);
   for (auto it = mVariables.begin(); it != mVariables.end(); ++it)
   {
      Poco::replaceInPlace(os, "$" + it->first, it->second);
      Poco::replaceInPlace(os, "${" + it->first+"}", it->second);
   }
   return os;
}


// -----------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------


persistvariables::persistvariables(std::string name, Poco::Path path, const std::vector<Configuration> config) : 
   mName(name), mPath(path), mConfig(config)
{
   // set default values for settings if they don't already have a setting.
   for (const auto & x : mConfig)
      if (!hasKey(x.name) || getVal(x.name).length() == 0)
         mVariables.setVal(x.name, x.defaultval);
}

cResult persistvariables::loadvariables()
{
   if (!utils::fileexists(mPath))
      return cError("The settings file does not exist: " + mPath.toString());

   // read the settings.
   variables storedvars;
   std::ifstream is(mPath.toString());
   if (is.bad())
      return cError("Unable to open " + mPath.toString() + " for reading.");
   try
   {
      cereal::JSONInputArchive archive(is);
      archive(storedvars);
   }
   catch (const cereal::Exception & e)
   {
      return cError("Cereal exception on reading settings: " + std::string(e.what()));
   }

   // check all items are (1) listed in config, and (2) valid.
   for (auto x : storedvars.getAll())
      setVal(x.first, x.second);
      
   return kRSuccess;
}

cResult persistvariables::savevariables() const
{
   logdbg("Creating " + mPath.toString());
   std::ofstream os(mPath.toString());
   if (os.bad() || !os.is_open())
      return cError("Unable to open " + mPath.toString() + " for writing.");
   try
   {
      cereal::JSONOutputArchive archive(os);
      archive(mVariables);
   }
   catch (const cereal::Exception & e)
   {
      return cError("Cereal exception on writing settings: " + std::string(e.what()));
   }
   drunner_assert(utils::fileexists(mPath), "Failed to create blank settings at " + mPath.toString());
   return kRSuccess;
}

cResult persistvariables::checkRequired() const
{
   for (const auto & x : mConfig)
      if (x.required && !hasKey(x.name))
         return cError("Required setting " + x.name + " is not defined.");
   return kRSuccess;
}

cResult persistvariables::setVal(std::string key, std::string val)
{
   for (const auto & x : mConfig)
      if (Poco::icompare(x.name, key) == 0)
      { // check valid given type!
         cResult r = _checkvalid(x.name, val, x);
         if (r.success())
            mVariables.setVal(x.name, val);
         else
            logdbg("Invalid setting: " + key + "=" + val + "\n" + r.what());
         return r;
      }
   return cError("Setting '" + key + "' is not recognised.");
}



std::string _pad(std::string x, unsigned int w)
{
   while (x.length() < w) x += " ";
   return x;
}
inline int _max(int a, int b) { return (a > b) ? a : b; }

cResult persistvariables::_showconfiginfo() const
{ // show current variables.
   logmsg(kLINFO, "Current configuration is:\n ");

   int maxkey = 0;
   for (const auto & y : mVariables.getAll())
      maxkey = _max(maxkey, y.first.length());
   for (const auto & y : mVariables.getAll())
      for (const auto & z : mConfig)
         if (Poco::icompare(z.name, y.first) == 0 && z.usersettable)
         {
            logmsg(kLINFO, " " + _pad(y.first, maxkey) + " = " + y.second);
            logmsg(kLINFO, " " + _pad(" ", maxkey) + "   " + z.description + "\n");
         }

   logmsg(kLINFO, " ");
   logmsg(kLINFO, "Change configuration variables with:");
   logmsg(kLINFO, " " + mName + " configure VARIABLE         -- configure from environment var");
   logmsg(kLINFO, " " + mName + " configure VARIABLE=VALUE   -- configure with specified value");
   return kRSuccess;
}

cResult persistvariables::handleConfigureCommand(CommandLine cl) 
{
   if (cl.args.size() == 0)
      return _showconfiginfo();

   cResult rval;
   for (const auto & kv : cl.args)
   {
      std::string key, val;
      size_t epos = kv.find('=');
      if (epos == std::string::npos)
      { // env variable
         try {
            key = kv;
            val = Poco::Environment::get(key);
         }
         catch (const Poco::Exception &)
         {
            logmsg(kLWARN, "Couldn't find environment variable " + key);
            logmsg(kLERROR, "Configuration variables must be given in form key=val or represent an environment variable.");
         }
         logmsg(kLINFO, "Setting " + key + " to value from environment [not logged].");
      }
      else
      { // form key=val.
         if (epos == 0)
            logmsg(kLERROR, "Missing key.");
         if (epos == kv.length() - 1)
            logmsg(kLERROR, "Missing value.");

         key = kv.substr(0, epos);
         val = kv.substr(epos + 1);
         logmsg(kLINFO, "Setting " + key + " to " + val);
      }

      for (const auto & x : mConfig)
         if (Poco::icompare(x.name, key) == 0)
            if (!x.usersettable)
               return cError("You can't override " + x.name);

      // find the corresponding configuration definition and set the variable.
      rval += setVal(key, val);
   }
   if (!rval.noChange())
      rval += savevariables();
   return rval;
}

void persistvariables::_addConfig(const Configuration & c)
{
   mConfig.push_back(c);
}

void persistvariables::setVal_mem(std::string key, std::string val)
{// ignore configuration, memory only (not persisted).
   return mVariables_Mem.setVal(key, val); 
}

bool persistvariables::hasKey(std::string key) const { 
   return mVariables.hasKey(key) || mVariables_Mem.hasKey(key); 
}

std::string persistvariables::getVal(std::string key) const 
{ 
   return mVariables.hasKey(key) ? mVariables.getVal(key) : mVariables_Mem.getVal(key); 
}

bool persistvariables::getBool(std::string key) const 
{ 
   return mVariables.hasKey(key) ? mVariables.getBool(key) : mVariables_Mem.getBool(key);
}

std::string persistvariables::substitute(std::string s) const
{ 
   return mVariables.substitute(mVariables_Mem.substitute(s)); 
}

const tKeyVals persistvariables::getAll() const 
{ 
   return getVariables().getAll();
}

const variables persistvariables::getVariables() const 
{ 
   return variables(mVariables, mVariables_Mem);
}

cResult persistvariables::_checkvalid(std::string key, std::string val, Configuration config)
{
   return kRSuccess;
}


