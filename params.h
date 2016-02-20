#include <vector>
#include <string>

#ifndef __PARAMS_H
#define __PARAMS_H

namespace params
{

   enum eCommand {
      c_UNDEFINED,
      c_setup,
      c_clean,
      c_list,
      c_update,
      c_checkimage,
      c_backup,
      c_restore,
      c_install,
      c_recover,
      c_uninstall,
      c_obliterate,
      c_enter,
      c_status
   };

   enum eOutputmode {
      om_normal,
      om_verbose,
      om_silent,
      om_getouput,   // get the output of the service. dRunner must be silent.
   };

   class params {
   public:
      params(int argc, char **argv);
      std::string substitute( const std::string & source ) const;

      const std::string & getVersion() const             {return mVersion;}
      eCommand getCommand() const                        {return mCmd;}
      bool isVerbose() const                             {return mOMode==om_verbose;}
      bool drIsSilent() const                            {return mOMode==om_silent || mOMode==om_getouput;}
      bool serviceIsSilent() const                       {return mOMode==om_silent;}
      const std::vector<std::string> & getArgs() const   {return mArgs;}

   private:
      std::string mVersion;
      eCommand mCmd;
      std::vector<std::string> mArgs;
      eOutputmode mOMode;
      eCommand parsecmd(std::string s) const;
      params();
   };

} // namespace

#endif
