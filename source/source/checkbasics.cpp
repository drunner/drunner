#include "utils.h"
#include "checkbasics.h"


// ---------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------
#ifdef _WIN32
// Windows

//std::string getUSER()
//{
//   char user_name[501];
//   DWORD user_name_size = sizeof(user_name);
//   if (!GetUserName(user_name, &user_name_size))
//      fatal("Couldn't get current user.");
//   return user_name;
//}

void check_basics()
{
}


// ---------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------
#else
// Linux

std::string getUSER()
{
   std::string op;
   if (0 != utils::bashcommand("echo $USER", op))
      logmsg(kLERROR, "Couldn't get current user. (" + op + ")");
   return op;
}

void check_basics()
{

   uid_t euid = geteuid();
   if (euid == 0)
      fatal("Please run as a standard user, not as root.");

   std::string user = utils::getUSER();
   if (0 != bashcommand("groups $USER | grep docker"))
      fatal("Please add the current user to the docker group. As root: " + kCODE_S + "adduser " + user + " docker" + kCODE_E);

   if (0 != bashcommand("groups | grep docker"))
      fatal(user + " hasn't picked up group docker yet. Log out then in again, or run " + kCODE_S + "exec su -l " + user + kCODE_E);

   if (!utils::commandexists("docker"))
      fatal("Please install Docker before using dRunner.\n(e.g. use  https://raw.githubusercontent.com/j842/scripts/master/install_docker.sh )");

   if (!utils::commandexists("curl"))
      fatal("Please install curl before using dRunner.");

   if (!utils::commandexists("docker-compose"))
      fatal("Please install docker-compose before using dRunner.");

   std::vector<std::string> args = { "--version" };
   if (utils::runcommand("docker", args) != 0)
      fatal("Running \"docker --version\" failed! Is docker correctly installed on this machine?");

   bool canRunDocker = utils::canrundocker(getUSER());
   logmsg(kLDEBUG, "Username: " + getUSER() + ",  Docker OK: " + (canRunDocker ? "YES" : "NO") + ", " + utils::get_exename() + " path: " + utils::get_exepath());
}


// ---------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------

#endif