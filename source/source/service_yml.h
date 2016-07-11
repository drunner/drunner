#ifndef __SERVICE_YML_H
#define __SERVICE_YML_H

#include <Poco/Path.h>



class serviceyml {
public:
   serviceyml(Poco::Path path);


private:
   std::vector<std::string> mVolumes;
};


#endif
