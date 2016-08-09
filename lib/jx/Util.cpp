#include "Util.h"
#include <boost/filesystem.hpp>

namespace util {

static std::string gExecutableDirectory;

void onStart(int argc, char* argv[]){
    gExecutableDirectory = boost::filesystem::system_complete(argv[0]).parent_path().string();
}

const std::string& executableDirectory(){
    return gExecutableDirectory;
}

}