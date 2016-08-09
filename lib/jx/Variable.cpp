#include "Variable.h"
#include "Util.h"
#include "VmMemory.h"
#include "StringUtils.h"

static std::string printStringContent(Variable v) {
    assert(v.type == ObjectRef);
    assert(v.value.object->type->name() == "java/lang/String");
    auto data = v.value.object->fields["java/lang/String__value"];
    if (data.value.object == nullptr){
        return "<uninitialized>";
    }
    assert(data.isArray());


    std::u16string dataCopy;
    dataCopy.resize(data.array()->length);
    for (size_t i = 0; i < data.array()->length; i++){
        dataCopy[i] = (uint16_t) data.array()->values[i].iv;
    }
    std::string utf8String = stringutils::utf16toUtf8(dataCopy);
    return utf8String;
}

std::string Variable::toString() const {
    if (type == None) {
        return "<none>";
    }
    std::ostringstream ss;
    ss << variableTypeToString(type);
    ss << "(";

    switch (type){
        case Boolean:
            ss << ((value.iv != 0) ? "true" : "false");
            break;
        case Integer:
        case Char:
        case Short:
            ss << value.iv;
            break;
        case ObjectRef:
        case ArrayRef:
            if (value.object == nullptr){
                ss << "null";
            } else {
               if (value.object->array){
                   ss << "Array[" << variableTypeToString(value.object->array->type) << " len=" << value.object->array->length << "]";
               } else {
                   ss << "[" << value.object->type->name() << "]";
                   if (value.object->type->name() == "java/lang/String"){
                       ss << " value=" << printStringContent(*this);
                   }
               }
            }
            break;
        case Double:
            ss << value.dv;
            break;
        case Float:
            ss << value.fv;
            break;
        case Long:
            ss << value.lv;
            break;
        default:
            ss << "unknown";
    }
    ss << ")";
    return ss.str();
}

bool Variable::isArray() const {
    return memoryType() == ObjectRef && (value.object == nullptr || value.object->array);
}

Array* Variable::array() {
    assert(isArray());
    return (value.object->array.get());
}

std::string Variable::stringValue() const {
    if (type != ObjectRef || value.object == nullptr || !value.object->type || value.object->type->name() != "java/lang/String"){
        throw std::invalid_argument("Not a string");
    }
    return printStringContent(*this);
}