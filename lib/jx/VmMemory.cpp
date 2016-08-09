#include "VmMemory.h"
#include "DescriptorParser.h"
#include "Log.h"

VmMemory::VmMemory() {

}

VmMemory::~VmMemory() {
    for (auto object : mObjects){
        delete object;
    }
}

Variable VmMemory::allocateObject(const std::shared_ptr<ClassFile> &type) {
    Object * object = new Object();
    object->type = type;

    mObjects.push_back(object);

    ClassFilePtr currentClass = type;
    while(currentClass){
        auto fields = currentClass->fields();
        for (const auto& field : fields){
            if (!field.isStatic){
                std::string name = field.isPrivate ? currentClass->name() + "__" + field.name : field.name;
                DescriptorParser descriptorParser (field.descriptor);
                object->fields[name].type = descriptorParser.type();
                logd("Initialized field ", name, " of new instance of ", type->name());
            }
        }
        currentClass = currentClass->superClassFile().lock();
    }

    return object;
}

Variable VmMemory::allocateArray(const VariableType &arrayType, size_t len) {
    Object * object = new Object();
    mObjects.push_back(object);
    object->array = std::shared_ptr<Array>(new Array(len, arrayType));

    Variable arrayReference (ArrayRef);
    arrayReference.value.object = object;
    return arrayReference;
}

Variable VmMemory::allocateObjectArray(size_t len, const std::string &descriptor) {
    logd("Allocate array of type ", descriptor);
    Object * object = new Object();
    object->array = std::shared_ptr<Array>(new Array(len, ObjectRef));
    object->array->objectType = descriptor;
    mObjects.push_back(object);
    Variable arrayReference (ArrayRef);
    arrayReference.value.object = object;
    return arrayReference;
}

Variable VmMemory::getGlobal(const GlobalVariableIdentifier &identifier) {
    const auto globalIt = mGlobals.find(identifier);
    if (globalIt == mGlobals.end()){
        throw std::invalid_argument("Global static " + identifier.toString() + " not found");
    }
    return globalIt->second;
}

void VmMemory::putGlobal(const GlobalVariableIdentifier &identifier, const Variable& value) {
    auto globalIt = mGlobals.find(identifier);
    if (globalIt == mGlobals.end()){
        throw std::invalid_argument("Could not find global referenced by" + identifier.toString());
    }
    if (globalIt->second.memoryType() != value.memoryType()){
        throw std::invalid_argument(std::string("Type of variable mismatch, expected ") + variableTypeToString(globalIt->second.type) + " found " + variableTypeToString(value.type));
    }
    globalIt->second.value = value.value;
}

void VmMemory::initGlobal(const GlobalVariableIdentifier &identifier, const VariableType &type) {
    mGlobals[identifier] = Variable(type);
}