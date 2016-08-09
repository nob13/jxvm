#include "Interpreter.h"
#include <functional>

struct MethodOverrideIdentifier {
    std::string className;
    std::string methodName;
    std::string description;

    std::size_t hash() const {
        return hash::combineHash(hash::combineHash(hash::stringHash(className), hash::stringHash(methodName)), hash::stringHash(description));
    }
};

inline bool operator==(const MethodOverrideIdentifier& a, const MethodOverrideIdentifier& b){
    return a.className == b.className && a.methodName == b.methodName && a.description == b.description;
}

struct FunctionContext {
    Interpreter * interpreter;
    ClassLoader * loader;
    VmMemory * memory;
    const Frame * previousFrame;
};

class MethodOverrides {
public:
    typedef std::function<Variable (const FunctionContext& context, const Variables&)> FunctionOverride;
    void add(const MethodOverrideIdentifier& id, const FunctionOverride& override){
        mOverrides[id] = override;
    }
    void add(const std::string& className, const std::string& methodName, const std::string& description, const FunctionOverride& override){
        MethodOverrideIdentifier id;
        id.className = className;
        id.methodName = methodName;
        id.description = description;
        add(id, override);
    }
    FunctionOverride find(const MethodOverrideIdentifier& id) const {
        auto it = mOverrides.find(id);
        return it == mOverrides.end() ? FunctionOverride() : it->second;
    }

    void addDefaultOverrides();
private:
    std::unordered_map<MethodOverrideIdentifier, FunctionOverride, hash::MethodHash<MethodOverrideIdentifier>> mOverrides;
};