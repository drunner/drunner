#include "service_yml.h"
#include "utils.h"

namespace serviceyml
{

   file::file(Poco::Path path, const variables & v) // reads file, applies variable substitution. Throws if bad.
   {
      mReadOkay = false;
      poco_assert(path.isFile());
      if (!utils::fileexists(path))
         return;
   }


} // namespace