#include "ClassLoader.h"
#include "Log.h"

std::string suffix (const std::string& string) {
    auto idx = string.find_last_of('.');
    if (idx == std::string::npos){
        return "";
    } else {
        return string.substr(idx + 1);
    }
}

std::shared_ptr<ClassFile> ClassLoader::loadByFile(const std::string& name) {
    BinaryReader reader(name);
    // TODO: Memory
    auto ptr = std::make_shared<ClassFile>(ClassFile::parse(reader));
    if (mClasses.count(ptr->name()) > 0){
        logi("Will overwrite existing instance of ", ptr->name());
    }
    mClasses[ptr->name()] = ptr;
    logi("Loaded ", ptr->name());
    // ptr->dump(std::cout);

    fillSuperClasses(*ptr);
    return ptr;
}


std::shared_ptr<ClassFile> ClassLoader::loadByName(const std::string& name){
    const auto i = mClasses.find(name);
    if (i != mClasses.end()){
        return i->second;
    }

    for (const auto& path : mPaths){
        // TODO
    }

    for (const auto& zipSource: mJars){
        ByteArrayPtr bytes = zipSource->findClassSource(name + ".class");
        if (bytes){
            BinaryReader reader(*bytes);
            // TODO Memory
            auto classFile = std::make_shared<ClassFile>(ClassFile::parse(reader));
            mClasses[name] = classFile;
            logi("Loaded ", name, " from ", zipSource->path());
            // classFile->dump(std::cout);
            fillSuperClasses(*classFile);
            return classFile;
        }
    }


    throw std::invalid_argument("Could not find class " + name);
}

void ClassLoader::addPath(const std::string& path) {
    if (suffix(path) == "jar"){
        std::shared_ptr<ZipSource> zipSource = std::make_shared<ZipSource>(path);
        mJars.push_back(zipSource);
    } else {
        mPaths.push_back(path);
    }
}

void ClassLoader::addDefaultPaths() {
    char * javaHome = getenv("JAVA_HOME");
    if (javaHome == nullptr) {
        throw std::runtime_error("JAVA_HOME not set");
    }
    addPath(util::executableDirectory() + "/../lib/main.jar");
    addPath(std::string(javaHome) + "/jre/lib/rt.jar");
}

void ClassLoader::fillSuperClasses(ClassFile & target) {
    auto current = target.superClassFile().lock();
    if (!current){
        auto superClassName = target.superClass();
        if (superClassName){
            auto superClassPtr = loadByName(superClassName.get());
            if (!superClassPtr){
                throw std::invalid_argument("Could not find super class " + superClassName.get());
            }
            target.setSuperClassFile(superClassPtr);
        }
    }
}

