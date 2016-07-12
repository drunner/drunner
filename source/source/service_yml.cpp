#include "service_yml.h"
#include "utils.h"

namespace serviceyml
{

   file::file(Poco::Path path, const variables & v) : simplefile(path)
   {
      mReadOkay = false;
      poco_assert(path.isFile());
      if (!utils::fileexists(path))
         return;
   }

   void file::getDockerVolumeNames(std::vector<std::string> & vols) const
   {
      poco_assert(vols.size() == 0);
      for (const auto & v : mVolumes)
         vols.push_back(v.name());
   }


} // namespace