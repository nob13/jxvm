#pragma once
#include "types.h"

struct Object;
struct Array;

enum VariableType {
    Integer, Float, Char, Byte, Short, Double, ObjectRef, ArrayRef, Boolean, Long, None
};

/** JVM understoods Boolean/Short as Integer under the hood. Also array are often treated as Objects. This returns the in-memory type of each type. */
inline VariableType returnMemoryType(VariableType type){
    switch (type){
        case Boolean:
        case Integer:
        case Char:
        case Short:
            return Integer;
        case Byte:
            return Byte;
        case ArrayRef:
        case ObjectRef:
            return ObjectRef;
        case Long:
            return Long;
        case Double:
            return Double;
        case None:
            return None;
        default:
            return type;
    }
}

inline const char* variableTypeToString(VariableType  type){
    switch(type){
        case Integer:
            return "Integer";
        case Float:
            return "Float";
        case Char:
            return "Char";
        case Byte:
            return "Byte";
        case Short:
            return "Short";
        case Double:
            return "Double";
        case ObjectRef:
            return "ObjectRef";
        case ArrayRef:
            return "ArrayRef";
        case Boolean:
            return "Boolean";
        case Long:
            return "Long";
        case None:
            return "None";
        default:
            return "Unknown";
    }
}

typedef union {
    int32_t iv;
    float fv;
    double dv;
    Object * object;
    // bool bv; // bools are stored as Integer
    // uint16_t cv;
    // int8_t bytev;
    // int16_t sv; // are stored as integer
    int64_t lv;
} ValueUnion;

inline ValueUnion defaultValue(VariableType type){
    ValueUnion result;
    switch (type){
        case Integer:
        case Boolean:
        case Short:
        case Byte:
        case Char:
            result.iv = 0;
            break;
        case Float:
            result.fv = 0.0f;
            break;
        case ObjectRef:
            result.object = nullptr;
            break;
        case ArrayRef:
            result.object = nullptr;
            break;
        // case Boolean:
        //    result.bv = false;
        //    break;
        case Double:
            result.dv = 0.0;
            break;
        // case Char:
        //    result.cv = 0;
            break;
        // case Byte:
        //    result.bytev = 0;
        //    break;
        // case Short:
        //    result.sv = 0;
        //    break;
        case Long:
            result.lv = 0;
            break;
        case None:
            result.object = nullptr;
            break;
        default:
            throw std::invalid_argument("Unknown type");
    }
    return result;
}

struct Variable {
    Variable() {
        type = None;
        value.object = nullptr;
    }
    Variable(int32_t iv){
        type = Integer;
        value.iv = iv;
    }
    Variable(float f){
        type = Float;
        value.fv = f;
    }
    Variable(Object* o){
        type = ObjectRef;
        value.object = o;
    }
    Variable(double d){
        type = Double;
        value.dv = d;
    }

    Variable(VariableType type){
        this->type = type;
        value = defaultValue(type);
    }

    bool isStoredAsInteger() const {
        return type == Short || type == Integer || type == Boolean || type == Char || type == Byte;
    }

    VariableType memoryType() const {
        return returnMemoryType(type);
    }

    bool isArray() const;

    Array* array();

    std::string toString() const;

    std::string stringValue() const;

    VariableType type;
    ValueUnion value;
};