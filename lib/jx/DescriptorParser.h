#pragma once

#include <string>
#include "Variable.h"

class DescriptorParser {

public:
    DescriptorParser(const std::string &descriptor);

    bool isMethod() const { return mIsMethod; }

    VariableType type() const { return mType; }

    int argumentCount() const { return mArgumentCount; }

    VariableType argument(int id) const;

private:
    VariableType mType;
    bool mIsMethod;
    int mArgumentCount;
    std::string mDescriptor;

};