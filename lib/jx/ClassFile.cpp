#include "ClassFile.h"
#include <stdexcept>
#include "Util.h"
#include <netinet/in.h>
#include <iostream>
#include "DescriptorParser.h"
#include "Log.h"

ClassFile ClassFile::parse(BinaryReader & reader) {
    ClassFile file;
    file.parseFromReader(reader);
    return file;
}

void ClassFile::parseFromReader(BinaryReader& reader) {
    reader.readInto(mHeader.magic);
    reader.readInto(mHeader.minor_version);
    reader.readInto(mHeader.major_version);
    reader.readInto(mHeader.constant_pool_count);
    
    // Constant pool count is one less than entries in the constant table
    if (mHeader.constant_pool_count == 0){
        throw std::invalid_argument("Constant pool count must be >0");
    }
    ConstantEntry startConstant;
    startConstant.tag = ConstantEntry::StartTag;
    mConstants.push_back(startConstant);
    
    for (int i = 1; i < mHeader.constant_pool_count; i++){
        // std::cout << "Parse constant entry " << (i + 1) << "/" << mHeader.constant_pool_count << std::endl;
        ConstantEntry constant = parseConstantEntry(reader);
        mConstants.push_back(constant);
        // std::cout << "Parsed constant " << ConstantEntry::tagToString(constant.tag) << std::endl;
        if (constant.takesTwoSlots()){
            ConstantEntry fill;
            fill.tag = ConstantEntry::FillTag;
            mConstants.push_back(fill);
            i++;
        }
    }
    reader.readInto(mHeader.access_flags);
    reader.readInto(mHeader.this_class);
    reader.readInto(mHeader.super_class);
    reader.readInto(mHeader.interfaces_count);
    
    for (int i = 0; i < mHeader.interfaces_count; i++){
        uint16_t interfaceIdx;
        reader.readInto(interfaceIdx);
        mInterfaces.push_back(interfaceIdx);
    }
    reader.readInto(mHeader.fields_count);
    
    for (int i = 0; i < mHeader.fields_count; i++){
        FieldInfo fieldInfo = parseFieldInfo(reader);
        mFieldInfos.push_back(fieldInfo);
    }
    
    reader.readInto(mHeader.methods_count);
    for (int i = 0; i < mHeader.methods_count; i++){
        MethodInfo methodInfo = parseMethodInfo(reader);
        mMethodInfos.push_back(methodInfo);
    }
    
    reader.readInto(mHeader.attributes_count);
    for (int i = 0; i < mHeader.attributes_count; i++){
        AttributeInfo attributeInfo = parseAttributeInfo(reader);
        mAttributeEntries.push_back(attributeInfo);
    }
    
    if(reader.rest() > 0){
        logw("there are ", reader.rest(), " unparsed bytes");
        
    }
    
    ConstantEntry classConstant = mConstants[mHeader.this_class];
    mName = getUtf8Constant(classConstant.nameIndex());

    for (uint32_t i = 0; i < mConstants.size(); i++){
        ConstantEntry entry = mConstants[i];
        if (entry.tag == ConstantEntry::Utf8Tag){
            if (getUtf8Constant(entry) == "Code"){
                mCodeIndex = i;
            }
        }
    }
}

AttributeInfo ClassFile::parseAttributeInfo(BinaryReader& reader) {
    AttributeInfo attributeInfo;
    reader.readInto(attributeInfo.attributeNameIndex);
    reader.readInto(attributeInfo.attributeLength);
    attributeInfo.byteIndex = reader.readBytesInto(mBytes, attributeInfo.attributeLength);
    return attributeInfo;
}

ConstantEntry ClassFile::parseConstantEntry(BinaryReader& reader){
    ConstantEntry constant = {0};
    reader.readInto(constant.tag);
    switch (constant.tag){
        case ConstantEntry::ClassTag:
        case ConstantEntry::StringTag:
        case ConstantEntry::MethodTypeTag:
            reader.readIntoBytes(constant.bytes, 2);
            break;
        case ConstantEntry::FieldRefTag:
        case ConstantEntry::MethodRefTag:
        case ConstantEntry::InterfaceMethodRefTag:
        case ConstantEntry::IntegerTag:
        case ConstantEntry::FloatTag:
        case ConstantEntry::NameAndTypeTag:
        case ConstantEntry::InvokeDynamicTag:
            reader.readIntoBytes(constant.bytes, 4);
            break;
        case ConstantEntry::DoubleTag:
        case ConstantEntry::LongTag:
            reader.readIntoBytes(constant.bytes, 8);
            break;
        case ConstantEntry::Utf8Tag: {
            uint16_t length;
            reader.readInto(length);
            
            uint32_t idx = reader.readBytesInto(mBytes, length);
            
            uint32_t length4 = length;
            
            memcpy(constant.bytes, &idx, 4);
            memcpy(constant.bytes + 4, &length4, 4);

            // std::string utf8String = getUtf8Constant(constant);
            // std::cout << "Parsed string " << utf8String << " len " << length << std::endl;

            break;
        }
        case ConstantEntry::MethodHandleTag:
            reader.readIntoBytes(constant.bytes, 3);
            break;
        default:
            throw std::invalid_argument("Unknown constant tag " + util::toString((int)constant.tag));
    }
    return constant;
}

FieldInfo ClassFile::parseFieldInfo(BinaryReader& reader) {
    FieldInfo fieldInfo;
    reader.readInto(fieldInfo.accessFlags);
    reader.readInto(fieldInfo.nameIdx);
    // TODO: Check Name Index validity
    reader.readInto(fieldInfo.descriptorIdx);
    // TODO: Check Descriptor Index validity
    reader.readInto(fieldInfo.attributeCount);
    for (int j = 0; j < fieldInfo.attributeCount; j++){
        AttributeInfo attributeInfo = parseAttributeInfo(reader);
        fieldInfo.attributes.push_back(attributeInfo);
    }
    return fieldInfo;
}

MethodInfo ClassFile::parseMethodInfo(BinaryReader& reader) {
    MethodInfo methodInfo;
    // TODO: Again, checking name index and method infdexes
    reader.readInto(methodInfo.accessFlags);
    reader.readInto(methodInfo.nameIdx);
    reader.readInto(methodInfo.descriptorIdx);
    reader.readInto(methodInfo.attributeCount);
    for (int j = 0; j < methodInfo.attributeCount; j++){
        AttributeInfo attributeInfo = parseAttributeInfo(reader);
        methodInfo.attributes.push_back(attributeInfo);
    }
    return methodInfo;
}

template<class T, class N> void dumpLine(std::ostream& stream, const N& name, const T& v){
    stream << "  " << name << "\t" << v << std::endl;
}

std::string ClassFile::getUtf8Constant(uint16_t idx) const {
    ConstantEntry entry = mConstants.at(idx);
    return getUtf8Constant(entry);
}

std::string ClassFile::getUtf8Constant(const ConstantEntry& entry) const {
    if (entry.tag != ConstantEntry::Utf8Tag){
        throw std::invalid_argument("Argument is not referring to UTF8 cosntant");
    }
    uint32_t byteIndex;
    uint32_t length;
    
    memcpy(&byteIndex, entry.bytes, 4);
    memcpy(&length, entry.bytes + 4, 4);
    
    const uint8_t * bytes = &mBytes[byteIndex];
    // TODO: JVM spec requires much more UTF8 processing
    std::string result (reinterpret_cast<const char*>(bytes), length);
    return result;
}

void ClassFile::dump(std::ostream &stream) const {
    stream << "Header: " << std::endl;

    dumpLine(stream, "Magic", util::toHex(mHeader.magic));
    dumpLine(stream, "Minor", mHeader.minor_version);
    dumpLine(stream, "Major", mHeader.major_version);
    dumpLine(stream, "Constant Pool Count", mHeader.constant_pool_count);
    for (uint32_t i = 0; i < mConstants.size(); i++){
        const auto& constant = mConstants[i];
        dumpLine(stream, "Constant " + util::toString(i), toString(constant));
    }
    
    
    dumpLine(stream, "Access Flags", accessRestrictionToString(mHeader.access_flags));
    dumpLine(stream, "This Class", mHeader.this_class);
    dumpLine(stream, "Super Class", mHeader.super_class);
    dumpLine(stream, "Interface Count", mHeader.interfaces_count);
    dumpLine(stream, "Fields Count", mHeader.fields_count);
    
    for (auto& field: mFieldInfos){
        dumpLine(stream, "  Field ", accessRestrictionToString(field.accessFlags));
        dumpLine(stream, "    Descriptor", getUtf8Constant(field.descriptorIdx));
        dumpLine(stream, "    Name", getUtf8Constant(field.nameIdx));
        for (auto& attribute: field.attributes){
            dumpLine(stream, "      Attribute ", toString(attribute));
        }
    }
    
    dumpLine(stream, "Methods Count", mHeader.methods_count);
    for (auto& method: mMethodInfos){
        dumpLine(stream, "  Method ", toString(method));
    }

    dumpLine(stream, "Attribute Count", mHeader.attributes_count);
    for (auto& attribute: mAttributeEntries){
        dumpLine(stream, "  Attribute ", toString(attribute));
    }
    dumpLine(stream, "Binary size", mBytes.size());
}

std::string ClassFile::toString(const ConstantEntry& entry) const {
    switch (entry.tag){
        case ConstantEntry::ClassTag:
            return "Class " + getUtf8Constant(entry.nameIndex());
        case ConstantEntry::MethodRefTag:{
            uint16_t classIndex = entry.classIndex();
            uint16_t nameAndType = entry.nameAndTypeIndex();

            std::string className = getUtf8Constant(mConstants[classIndex].nameIndex());
            const ConstantEntry& nameAndTypeConstant = mConstants[nameAndType];
            
            std::string methodName = getUtf8Constant(nameAndTypeConstant.nameIndex());
            std::string typeName   = getUtf8Constant(nameAndTypeConstant.descriptorIndex());
            
            return "MethodRef " + typeName + " " + className + "::" + methodName;
            // return "MethodRef, classIndex=" + util::toString(classIndex);
        }
        case ConstantEntry::NameAndTypeTag: {
            std::string methodName = getUtf8Constant(entry.nameIndex());
            std::string typeName   = getUtf8Constant(entry.descriptorIndex());
            return "NameAndType " + typeName + " " + methodName;
        }
        case ConstantEntry::Utf8Tag:
            return std::string("Utf8 ") + getUtf8Constant(entry);
        case ConstantEntry::FieldRefTag:{
            uint16_t classIndex = entry.classIndex();
            uint16_t nameAndTypeIndex = entry.nameAndTypeIndex();

            std::string className = getUtf8Constant(mConstants[classIndex].nameIndex());

            const ConstantEntry& nameAndTypeConstant = mConstants[nameAndTypeIndex];

            std::string fieldName = getUtf8Constant(nameAndTypeConstant.nameIndex());
            std::string typeName   = getUtf8Constant(nameAndTypeConstant.descriptorIndex());
            return "FieldRef " + className + " " + fieldName + " " + typeName;
        }
        case ConstantEntry::InterfaceMethodRefTag:{
            uint16_t classIndex = entry.classIndex();
            uint16_t nameAndTypeIndex = entry.nameAndTypeIndex();
            std::string className = getUtf8Constant(mConstants[classIndex].nameIndex());
            const ConstantEntry& nameAndTypeConstant = mConstants[nameAndTypeIndex];

            std::string methodName = getUtf8Constant(nameAndTypeConstant.nameIndex());
            std::string typeName   = getUtf8Constant(nameAndTypeConstant.descriptorIndex());

            return "InterfaceMethodRef " + typeName + " " + className + "::" + methodName;
        }
        default:
            return ConstantEntry::tagToString(entry.tag);
    }
}

boost::optional<MethodInfo> ClassFile::mainMethod() const {
    return methodWithName("main", Flags::PUBLIC | Flags::STATIC);
}

boost::optional<MethodInfo> ClassFile::clinit() const {
    return methodWithName("<clinit>", Flags::STATIC);
}

boost::optional<MethodInfo> ClassFile::methodWithName(const std::string& searchedName, int requiredFlags) const {
    for (auto i = mMethodInfos.begin(); i != mMethodInfos.end(); i++){
        const auto & mi = *i;
        if (requiredFlags == 0 || (mi.accessFlags & requiredFlags)) {
            auto name = getUtf8Constant(mi.nameIdx);
            if (name == searchedName) {
                return boost::optional<MethodInfo>(mi);
            }
        }
    }
    return boost::optional<MethodInfo>();
}

boost::optional<FieldInfo> ClassFile::fieldWithName(const std::string& searchedName) const {
    for (auto i = mFieldInfos.begin(); i != mFieldInfos.end(); i++){
        const auto & mi = *i;
        auto name = getUtf8Constant(mi.nameIdx);
        if (name == searchedName) {
            return boost::optional<FieldInfo>(mi);
        }
    }
    return boost::optional<FieldInfo>();
}


boost::optional<MethodInfo> ClassFile::methodWithSignature(const MethodIdentifier& identifier) const {
    for (const auto& method: mMethodInfos){
        if (getUtf8Constant(method.nameIdx) == identifier.methodName && getUtf8Constant(method.descriptorIdx) == identifier.descriptor) {
            return boost::optional<MethodInfo>(method);
        }
    }
    return boost::optional<MethodInfo>();
}


CodeIdentifier ClassFile::codeForMethod(const MethodInfo& method) const {
    if (method.accessFlags & Flags::NATIVE){
        std::string methodName = getUtf8Constant(method.nameIdx);
        throw std::invalid_argument("Method " + name() + " " + methodName + " is native");
    }
    logd("This class ",  name());
    std::vector<AttributeInfo>::const_iterator i = method.attributes.begin();
    logd("Code Index ", mCodeIndex);
    for (; i != method.attributes.end(); i++){
        logd(" Attribute name index ", i->attributeNameIndex);
        if (i->attributeNameIndex == mCodeIndex){
            break;
        }
    }

    if (i == method.attributes.end()){
        std::string methodName = getUtf8Constant(method.nameIdx);
        throw std::invalid_argument("Method " + name() + " " + methodName + " has no code block, abstract?");
    }
    const AttributeInfo& attributeInfo = *i;
    ByteRange range = ByteRange(mBytes, attributeInfo.byteIndex, attributeInfo.attributeLength);
    CodeIdentifier code;
    code.maxStack = range.readUint16();
    code.maxLocals = range.readUint16();
    code.codeLength = range.readUint32();
    if (code.codeLength > 65536){
        // See spec
        throw std::invalid_argument("Code length too long?");
    }
    code.code = range.subRangeUpTo(code.codeLength);

    return code;
}

std::string ClassFile::findClass(uint16_t index) const {
    const auto& constant = mConstants[index];
    if (constant.tag != ConstantEntry::ClassTag){
        throw std::invalid_argument("Index doesn't refer to class entry");
    }
    return getUtf8Constant(constant.nameIndex());
}

MethodIdentifier ClassFile::findMethod(uint16_t index) const {
    const auto& constant = mConstants[index];
    if (constant.tag != ConstantEntry::MethodRefTag){
        throw std::invalid_argument("Index doesn't refer to MethodRef entry");
    }
    MethodIdentifier identifier;

    const auto& classReferer = mConstants[constant.classIndex()];
    const auto& nameAndType = mConstants[constant.nameAndTypeIndex()];
    identifier.methodName = getUtf8Constant(nameAndType.nameIndex());
    identifier.descriptor = getUtf8Constant(nameAndType.descriptorIndex());
    identifier.className = getUtf8Constant(classReferer.nameIndex());
    return identifier;
}

MethodIdentifier ClassFile::findInterfaceMethod(uint16_t index) const {
    const auto& constant = mConstants[index];
    if (constant.tag != ConstantEntry::InterfaceMethodRefTag){
        throw std::invalid_argument("Index doesn't refer to a IndexMethodRef tag");
    }

    MethodIdentifier identifier;

    const auto& classReferer = mConstants[constant.classIndex()];
    const auto& nameAndType = mConstants[constant.nameAndTypeIndex()];
    identifier.methodName = getUtf8Constant(nameAndType.nameIndex());
    identifier.descriptor = getUtf8Constant(nameAndType.descriptorIndex());
    identifier.className = getUtf8Constant(classReferer.nameIndex());
    return identifier;
}

FieldRefIdentifier ClassFile::findFieldRefIdentifier(uint16_t index) const {
    const auto& constant = mConstants[index];
    if (constant.tag != ConstantEntry::FieldRefTag){
        throw std::invalid_argument("Index doesn't refer to FieldRef entry");
    }
    FieldRefIdentifier identifier;

    const auto& classReferer = mConstants[constant.classIndex()];
    const auto& nameAndType = mConstants[constant.nameAndTypeIndex()];

    identifier.className = getUtf8Constant(classReferer.nameIndex());
    identifier.fieldName = getUtf8Constant(nameAndType.nameIndex());
    identifier.descriptor = getUtf8Constant(nameAndType.descriptorIndex());
    return identifier;
}

std::vector<FieldInformation> ClassFile::fields() const {
    std::vector<FieldInformation> result;
    for (const auto& info: mFieldInfos){
        FieldInformation returnInfo;
        returnInfo.descriptor = getUtf8Constant(info.descriptorIdx);
        returnInfo.name = getUtf8Constant(info.nameIdx);
        returnInfo.isStatic = (bool)(info.accessFlags & Flags::STATIC);
        returnInfo.isPrivate = (bool)(info.accessFlags & Flags::PRIVATE);
        result.push_back(returnInfo);
    }
    return result;
}


std::string ClassFile::toString(const MethodInfo &entry) const {
    std::ostringstream stream;
    std::string name = getUtf8Constant(entry.nameIdx);
    std::string attributes = accessRestrictionToString(entry.accessFlags);
    std::string descriptor = getUtf8Constant(entry.descriptorIdx);
    stream << attributes << name << " " << " NameIdx: " << entry.nameIdx << " DescriptorIdx: " << entry.descriptorIdx << descriptor;

    for (const auto& attr : entry.attributes){
        stream << std::endl << "     Attribute " << toString(attr);
    }

    return stream.str();
}

std::string ClassFile::toString(const AttributeInfo &entry) const {
    std::stringstream stream;
    stream << getUtf8Constant(entry.attributeNameIndex) << " len=" << entry.attributeLength;
    return stream.str();
}
















