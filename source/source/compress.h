#ifndef __COMPRESS_H
#define __COMPRESS_H

#include <string>
#include <Poco/Path.h>

#include "params.h"

namespace compress
{
   bool compress_volume(std::string password, std::string volumename, Poco::Path archive);

   bool compress_folder(std::string password, Poco::Path foldername, Poco::Path archive);

   bool decompress_volume(std::string password, std::string targetvolumename, Poco::Path archive);

   bool decompress_folder(std::string password, Poco::Path targetfoldername, Poco::Path archive);
} // namespace

#endif