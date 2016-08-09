#include <iostream>
#include <string>
#include <jx/ClassFile.h>
#include <jx/Interpreter.h>
#include <jx/Util.h>

int main(int argc, char* argv[]) {
    if (argc != 2){
        std::cout << "Usage " << argv[0] << " <class-file>" << std::endl;
        return 1;
    }
    util::onStart(argc, argv);

    std::string classFileName = argv[1];


    Interpreter interpreter;
    interpreter.classLoader().addDefaultPaths();
    interpreter.executeFile(classFileName);

    return 0;
}