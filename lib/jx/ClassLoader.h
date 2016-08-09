#pragma once
#include "ClassFile.h"
#include <memory>
#include <map>
#include <unordered_map>
#include <zip.h>
#include "types.h"
#include <iostream>

class ZipSource : public boost::noncopyable{
public:
    ZipSource(const std::string& path){
        mPath = path;

        int err = 0;
        mZipHandle = zip_open(path.c_str(), 0, &err);
        if (err){
            throw std::invalid_argument("Could not open " + path);
        }
        // std::cout << "Opened " << path << std::endl;
    }

    ~ZipSource(){
        zip_close(mZipHandle);
    }

    const std::string& path() const { return mPath; }

    ByteArrayPtr findClassSource(const std::string& name) {
        struct zip_stat stat;
        zip_stat_init(&stat);
        zip_stat(mZipHandle, name.c_str(), 0, &stat);

        if (!stat.valid){
            return ByteArrayPtr();
        }
        ByteArrayPtr ptr = std::make_shared<ByteArray>();
        ptr->resize (stat.size);
        zip_file * file = zip_fopen(mZipHandle, name.c_str(), 0);
        if (!file) {
            return ByteArrayPtr();
        }
        zip_fread(file, &(*ptr->begin()), stat.size);
        zip_fclose(file);
        return ptr;
    }



private:
    std::string mPath;
    struct zip * mZipHandle;

};

class ClassLoader {
public:
    std::shared_ptr<ClassFile> loadByFile(const std::string& name);

    std::shared_ptr<ClassFile> loadByName(const std::string& name);

    void addPath(const std::string& path);

    void addDefaultPaths();

private:
    void fillSuperClasses(ClassFile& target);

    std::vector<std::string> mPaths;
    std::vector<std::shared_ptr<ZipSource> > mJars;
    // Sorted by Java Name
    std::unordered_map<std::string, std::shared_ptr<ClassFile>> mClasses;
};