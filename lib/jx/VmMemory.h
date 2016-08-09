#pragma once
#include "Variable.h"
#include "ClassFile.h"
#include "types.h"

struct Array {
    Array(uint32_t length, VariableType type){
        this->length = length;
        this->type = type;
        ValueUnion proto = defaultValue(type);
        this->values.resize(this->length, proto);
    }

    uint32_t length;
    VariableType type;

    VariableType memoryType() const {
        return returnMemoryType(type);
    }

    std::string objectType; // Type of the objects (if objectRef)

    static VariableType fromArrayTypeCode(uint8_t typeCode){
        switch (typeCode){
            case 4: return Boolean;
            case 5: return Char;
            case 6: return Float;
            case 7: return Double;
            case 8: return Byte;
            case 9: return Short;
            case 10: return Integer;
            case 11: return Long;
            default:
                throw std::invalid_argument("Unknown array type code " + util::toString(typeCode));
        }
    }

    std::vector<ValueUnion> values;
};

struct Object {
    // Class Type?
    std::shared_ptr<ClassFile> type;

    // Variables
    std::unordered_map<std::string, Variable> fields;

    Variable * publicField (const std::string & name ) {
        auto it = fields.find(name);
        return it == fields.end() ? nullptr : &it->second;
    }

    Variable * privateField (const std::string& className, const std::string & name) {
        std::string fullName = className + "__" + name;
        auto it = fields.find(fullName);
        return it == fields.end() ? nullptr : &it->second;
    }

    std::shared_ptr<Array> array;

};


struct GlobalVariableIdentifier {
    std::string className;
    std::string name;

    std::string toString() const { return className + "::" + name; }

    std::size_t hash() const { return hash::combineHash(hash::stringHash(className), hash::stringHash(name)); }
};

inline bool operator< (const GlobalVariableIdentifier& a, const GlobalVariableIdentifier& b){
    return a.className == b.className ? a.name < b.name : a.className < b.className;
}

inline bool operator==(const GlobalVariableIdentifier& a, const GlobalVariableIdentifier& b){
    return a.className == b.className && a.name == b.name;
}

/** Handles Heap Memory. */
class VmMemory {
public:
    VmMemory();
    ~VmMemory();

    Variable allocateObject(const std::shared_ptr<ClassFile>& type);
    Variable allocateArray(const VariableType& arrayType, size_t len);
    Variable allocateObjectArray(size_t len, const std::string& descriptor);

    void putGlobal(const std::string& className, const std::string& variableName, const Variable& value) {
        putGlobal(GlobalVariableIdentifier { className, variableName }, value);
    }
    void putGlobal(const GlobalVariableIdentifier& identifier, const Variable& value);
    Variable getGlobal(const std::string& className, const std::string& variableName) {
        return getGlobal(GlobalVariableIdentifier { className, variableName });
    }
    void initGlobal(const std::string& className, const std::string& variableName, const VariableType& type){
        initGlobal(GlobalVariableIdentifier { className, variableName }, type);
    }
    void initGlobal(const GlobalVariableIdentifier& identifier, const VariableType& type);


    Variable getGlobal(const GlobalVariableIdentifier& identifier);
private:
    std::vector<Object*> mObjects;
    std::unordered_map<GlobalVariableIdentifier, Variable, hash::MethodHash<GlobalVariableIdentifier>> mGlobals;
};