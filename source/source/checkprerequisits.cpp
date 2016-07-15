#include "utils.h"
#include "checkprerequisits.h"
#include "globallogger.h"

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

void check_prerequisits()
{
   std::vector<std::string> args = { "--version" };
   if (utils::runcommand("docker", args) != 0)
      fatal("Running \"docker --version\" failed! Is docker correctly installed on this machine?");
}


// ---------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------
#else
// Linux

#include <unistd.h>

int bashcommand(std::string bashline, std::string & op, bool trim)
{
   std::vector<std::string> args = { "-c", bashline };
   return runcommand("/bin/bash", args, op, trim);
}

int bashcommand(std::string bashline)
{
   std::string op;
   return bashcommand(bashline, op, false);
}


bool commandexists(std::string command)
{
   return (0 == bashcommand("command -v " + command));
}

std::string getUSER()
{
   std::string op;
   if (0 != bashcommand("echo $USER", op, true))
      logmsg(kLERROR, "Couldn't get current user. (" + op + ")");
   return op;
}

void check_prerequisits()
{

   uid_t euid = geteuid();
   if (euid == 0)
      fatal("Please run as a standard user, not as root.");

   std::string user = getUSER();
   if (0 != bashcommand("groups $USER | grep docker"))
      fatal("Please add the current user to the docker group. As root: " + utils::kCODE_S + "adduser " + user + " docker" + utils::kCODE_E);

   if (0 != bashcommand("groups | grep docker"))
      fatal(user + " hasn't picked up group docker yet. Log out then in again, or run " + utils::kCODE_S + "exec su -l " + user + utils::kCODE_E);

   if (!commandexists("docker"))
      fatal("Please install Docker before using dRunner.\n(e.g. use  https://raw.githubusercontent.com/j842/scripts/master/install_docker.sh )");

   if (!commandexists("curl"))
      fatal("Please install curl before using dRunner.");

   std::vector<std::string> args = { "--version" };
   if (utils::runcommand("docker", args) != 0)
      fatal("Running \"docker --version\" failed! Is docker correctly installed on this machine?");
}


// ---------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------------

#endif