#ifndef __COMPRESS_H
#define __COMPRESS_H

#include <string>

#include "params.h"

namespace compress
{
   bool compress_volume(std::string password, std::string volumename, 
                     std::string archivefolder, std::string archivename, const params & p);

   bool compress_folder(std::string password, std::string foldername,
      std::string archivefolder, std::string archivename, const params & p);



} // namespace

#endif