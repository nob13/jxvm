#include "DescriptorParser.h"
#include <stdexcept>

static VariableType mapToVariableType(char var) {
    switch (var) {
        case 'I':
            return VariableType::Integer;
        case 'B':
            return VariableType::Byte;
        case 'C':
            return VariableType::Char;
        case 'D':
            return VariableType::Double;
        case 'F':
            return VariableType::Float;
        case 'J':
            return VariableType::Long;
        case 'L':
            return VariableType::ObjectRef;
        case 'S':
            return VariableType::Short;
        case 'Z':
            return VariableType::Boolean;
        case '[':
            return VariableType::ArrayRef;
            //ToDo add Void
        case 'V':
            return VariableType::None;
        default:
            return VariableType::None;
    }
}

static void moveToEndOfObjectArgument(std::string::const_iterator * it, const std::string& s){
    while (*it != s.end()){
        if ((**it) == ';'){
            return;
        }
        (*it)++;
    }
    throw std::invalid_argument("No end of object argument in " + s);
}

static void moveToEndOfArrayArgument(std::string::const_iterator * it, const std::string& s){
    while (*it != s.end()){
        if ((**it) != '['){
            if ((**it) == 'L'){
                // workaround, arrays of objects
                moveToEndOfObjectArgument(it, s);
            }
            return;
        }
        (*it)++;
    }
    throw std::invalid_argument("No end of object argument in " + s);
}


DescriptorParser::DescriptorParser(const std::string &descriptor) {
    mDescriptor = descriptor;
    if (mDescriptor.empty()){
        throw std::invalid_argument("Empty descriptor");
    }

    mIsMethod = mDescriptor[0] == '(' && mDescriptor.find(')') != std::string::npos;
    auto endOfMethod = mDescriptor.find(')');
    if (endOfMethod == mDescriptor.npos){
        endOfMethod = 0;
    } else {
        endOfMethod++;
    }
    mType = mapToVariableType(mDescriptor.at(endOfMethod)); // at if endOfMethod is out of range

    mArgumentCount = 0;
    for (std::string::const_iterator it = mDescriptor.begin() + 1; it != mDescriptor.end(); it++){
        char c = *it;
        if (c == 'L'){
            moveToEndOfObjectArgument(&it, mDescriptor);
        }
        if (c == '['){
            moveToEndOfArrayArgument(&it, mDescriptor);
        }
        if (c == ')'){
            break;
        }
        mArgumentCount++;
    }
}

VariableType DescriptorParser::argument(int id) const {
    if (id > argumentCount() || id < 0) {
        throw std::invalid_argument("ID out of bounds");
    }
    int tmpId = 0;
    for (auto it = mDescriptor.begin() + 1; it != mDescriptor.end(); it++){
        char c = *it;
        if (tmpId == id){
            return mapToVariableType(c);
        }
        if (c == 'L'){
            moveToEndOfObjectArgument(&it, mDescriptor);
        }
        if (c == '['){
            moveToEndOfArrayArgument(&it, mDescriptor);
        }
        tmpId++;
    }
    return VariableType::None;
}