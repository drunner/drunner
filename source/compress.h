#ifndef __COMPRESS_H
#define __COMPRESS_H

#include <string>

namespace compress
{
   bool compress_volume(std::string password, std::string volumename, 
                     std::string archivefolder, std::string archivename);



} // namespace

#endif