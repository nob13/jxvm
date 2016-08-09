#pragma once
#include <memory>
#include "ClassLoader.h"
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include "Variable.h"
#include "Util.h"
#include "VmMemory.h"
#include "DescriptorParser.h"

// Some variables (e.g. Frame / Heap / Argument list).
struct Variables {
    std::vector<Variable> variables;

    bool empty() const { return variables.empty(); }

    // stack operations
    void push(const Variable& variable) { variables.push_back(variable); }

    size_t size() const { return variables.size(); }

    const Variable& top() const { return *variables.rbegin(); }

    /** Returns objects indexed from the top of the stack. */
    const Variable& top(size_t idx) { return *(variables.rbegin() + idx) ; }

    void popMany(size_t count) {
        if (count > variables.size()){
            throw std::invalid_argument("Count bigger than vector size");
        }
        variables.resize(variables.size() - count);
    }

    std::vector<Variable> popAndReturnMany(size_t count){
        if (count > variables.size()){
            throw std::invalid_argument("Count bigger than vector size");
        }
        std::vector<Variable> result (variables.end() - count, variables.end());
        variables.resize(variables.size() - count);
        return result;
    }

    Variable pop() { Variable v = top(); variables.pop_back(); return v; }
};

struct Frame {
    Object *thisp = 0;

    // TODO, necessary?
    void ensureLocalArraySpace(int idx);

    Variables stack;
    Variables localArray;
};

class MethodOverrides;

class JvmException : public std::exception {
public:
    JvmException (Variable exceptionObject) : mExceptionObject(exceptionObject) {
    }
    Variable exceptionObject () const { return mExceptionObject; }

    const char * what() const noexcept {
        return "Jvm Exeption";
    }

private:
    Variable mExceptionObject;
};

class Interpreter {
public:
    Interpreter();
    ClassLoader& classLoader() { return mClassLoader; }

    /** Execute a given file */
    void executeFile(const std::string& filename);

    void executeMain(const ClassFile& clazz);
    Variable executeMethod(const ClassFile& clazz, const MethodInfo& method, const Frame& previousFrame, const Variables& arguments);

    std::pair<ClassFilePtr, MethodInfo> virtualMethodDispatch(const MethodIdentifier& method, const Variable& thisPointer);

    Variable mainThread() const { return mMainThread; }

    // Convenience, call a static method
    Variable callStatic(const std::string& className, const std::string& methodName, const Variables& arguments);

    Variable classByName(const std::string& clazzName);

    ClassFilePtr findInitializedClass(const std::string& name);

private:
    void handleReturn(Frame* frame, Variable returnValue, const DescriptorParser& methodSignature);


    void prepareClazz(const ClassFilePtr& clazz);
    void initClass(const ClassFilePtr& clazz);

    Variable initializeString(const std::string& content, const Frame& previousFrame);

    void createMainThread();

    ClassLoader mClassLoader;

    std::unordered_set<std::string> mInitializedClasses;
    std::shared_ptr<MethodOverrides> mMethodOverrides;
    VmMemory mMemory;

    uint64_t mInstructionCount;

    Variable mMainThread;
};