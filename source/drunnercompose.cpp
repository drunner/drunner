#include <sstream>
#include <fstream>

#include "drunnercompose.h"
#include "utils.h"
#include "logmsg.h"
#include "service.h"

//void DockerCompose_Create(std::string inputfile, std::string outputfile, const service & svc, const params & p)
//{
//   if (!utils::fileexists(inputfile))
//      logmsg(kLERROR, "Couldn't find Docker Compose file " + inputfile, p);
//
//   std::ifstream t(inputfile);
//   std::stringstream buffer;
//   buffer << t.rdbuf();
//   std::string dc(buffer.str());
//   
//   dc = utils::replacestring(dc, "${SERVICENAME}", svc.getName());
//
//   std::ofstream out(outputfile);
//   if (!out.is_open())
//      logmsg(kLERROR, "Couldn't write Docker Compose file to " + outputfile, p);
//   out << dc;
//   out.close();
//}

void InstallDockerCompose(const params & p)
{

   std::string url("https://github.com/docker/compose/releases/download/1.6.2/docker-compose-Linux-x86_64");
   std::string trgt(utils::get_usersbindir() + "/docker-compose");

   if (utils::fileexists(trgt))
      utils::delfile(trgt, p);
   utils::downloadexe(url, trgt, p);
}
