#pragma once

#include <string>
#include <vector>
#include <sstream>
#include "BinaryReader.h"
#include "Util.h"
#include <boost/optional.hpp>
#include "ByteRange.h"


/* Main header of a class file. */
struct ClassFileHeader {
    uint32_t             magic;
    uint16_t             minor_version;
    uint16_t             major_version;
    uint16_t             constant_pool_count;

    uint16_t             access_flags;
    uint16_t             this_class;
    uint16_t             super_class;
    uint16_t             interfaces_count;

    uint16_t             fields_count;
    uint16_t             methods_count;

    uint16_t             attributes_count;
};

struct ConstantEntry {
    uint8_t tag;
    uint8_t bytes[8];


    // Note: special meaning for UTF8, just saving index into mBytes into the first 4 bytes
    // Length is saved in the next4 bytes, both in platform order

    static const int StartTag = 0; // Aritficial null entry, so that indexing start from 0
    static const int FillTag = 100; // Artificial fill entry for double-slot-taking entries.
    static const int ClassTag = 7;
    static const int FieldRefTag = 9;
    static const int MethodRefTag = 10;
    static const int InterfaceMethodRefTag = 11;
    static const int StringTag = 8;
    static const int IntegerTag = 3;
    static const int FloatTag = 4;
    static const int LongTag = 5;
    static const int DoubleTag = 6;
    static const int NameAndTypeTag = 12;
    static const int Utf8Tag = 1;
    static const int MethodHandleTag = 15;
    static const int MethodTypeTag = 16;
    static const int InvokeDynamicTag = 18;

    /** Some constant entries take two slots. */
    bool takesTwoSlots() const {
        return tag == DoubleTag || tag == LongTag;
    }


    /** Index of a name, valid for classes. */
    uint16_t nameIndex() const {
        assert (tag == ClassTag || tag == StringTag || tag == NameAndTypeTag);
        return ntohs(*reinterpret_cast<const uint16_t*>(bytes));
    }
    
    uint16_t classIndex() const {
        assert (tag == MethodRefTag || tag == InterfaceMethodRefTag || tag == FieldRefTag);
        return ntohs(*reinterpret_cast<const uint16_t*>(bytes));
    }
    
    uint16_t nameAndTypeIndex() const {
        assert (tag == MethodRefTag || tag == InterfaceMethodRefTag || tag == FieldRefTag);
        return ntohs(*reinterpret_cast<const uint16_t*>(bytes + 2));
    }
    
    uint16_t descriptorIndex() const {
        assert (tag == NameAndTypeTag);
        return ntohs(*reinterpret_cast<const uint16_t*>(bytes + 2));
    }

    int32_t integerValue() const {
        return ntohl(*reinterpret_cast<const int32_t*>(bytes));
    }

    int64_t longValue() const {
        return ntohll(*reinterpret_cast<const int64_t*>(bytes));
    }

    float floatValue() const {
        int32_t iv = integerValue();
        return reinterpret_cast<float&>(iv);
    }

    double doubleValue() const {
        int64_t lv = longValue();
        return reinterpret_cast<double&>(lv);
    }


    static const char * tagToString(uint8_t tag) {
        switch (tag){
            case StartTag: return "<start>";
            case FillTag: return "<fill>";
            case ClassTag: return "class";
            case FieldRefTag: return "fieldRef";
            case MethodRefTag: return "methodRef";
            case InterfaceMethodRefTag: return "interfaceMethod";
            case StringTag: return "string";
            case IntegerTag: return "integer";
            case FloatTag: return "float";
            case LongTag: return "long";
            case DoubleTag: return "double";
            case NameAndTypeTag: return "nameAndType";
            case Utf8Tag: return "utf8";
            case MethodHandleTag: return "methodHandle";
            case MethodTypeTag: return "methodType";
            case InvokeDynamicTag: return "invokeDynamic";
            default:
                return "UNKNOWN";
        }
    }
};

struct AttributeInfo {
    uint16_t attributeNameIndex;
    uint32_t attributeLength;
    // Start of the bytes
    size_t byteIndex;
};

struct FieldInfo {
    uint16_t accessFlags;
    uint16_t nameIdx;
    uint16_t descriptorIdx;
    uint16_t attributeCount;
    std::vector<AttributeInfo> attributes;
};


namespace Flags {
    const int PUBLIC = 0x0001;
    const int PRIVATE = 0x0002;
    const int PROTECTED = 0x0004;
    const int STATIC = 0x0008;
    const int FINAL = 0x0010;
    const int SUPER_SYNCHRONIZED = 0x0020;
    const int BRIDGE = 0x0040;
    const int VARAGS = 0x0080;
    const int NATIVE = 0x0100;
    const int STRICTFP = 0x0100;
    const int INTERFACE = 0x0200;
    const int ABSTRACT = 0x0400;
    const int SYNTHETIC = 0x1000;
    const int ANNOTATION = 0x2000;
    const int ENUM = 0x4000;
};


struct MethodInfo {
    uint16_t accessFlags;
    uint16_t nameIdx;
    uint16_t descriptorIdx;
    uint16_t attributeCount;
    std::vector<AttributeInfo> attributes;

    bool isNative() const { return (bool)(accessFlags & Flags::NATIVE); }
};

struct CodeIdentifier {
    uint16_t maxStack;
    uint16_t maxLocals;
    uint32_t codeLength;
    ByteRange code;

    // TODO: Exception table.
    // Atributes etc.

    std::string toString() const {
        return "Code maxStack=" + util::toString(maxStack) + " maxLocals=" + util::toString(maxLocals) + " length=" + util::toString(codeLength);
    }
};


static std::string accessRestrictionToString(uint16_t access){
    std::ostringstream result;
    if (access & Flags::PUBLIC) result << "public ";
    if (access & Flags::PRIVATE) result << "private ";
    if (access & Flags::PROTECTED) result << "protected ";
    if (access & Flags::STATIC) result << "static ";
    if (access & Flags::FINAL) result << "final ";
    if (access & Flags::SUPER_SYNCHRONIZED) result << "super/synchronized ";
    if (access & Flags::BRIDGE) result << "bridge "; // methods only
    if (access & Flags::VARAGS) result << "varargs "; // methods only
    if (access & Flags::NATIVE) result << "native "; // methods only
    if (access & Flags::STRICTFP) result << "strictfp "; // methods only
    
    if (access & Flags::INTERFACE) result << "interface ";
    if (access & Flags::ABSTRACT) result << "abstract ";
    if (access & Flags::SYNTHETIC) result << "synthetic ";
    if (access & Flags::ANNOTATION) result << "annotation ";
    if (access & Flags::ENUM) result << "enum ";
    return result.str();
}

struct MethodIdentifier {
    std::string className;
    std::string methodName;
    std::string descriptor;

    std::string toString() const { return descriptor + " " + className + " " + methodName;  }
};

struct FieldRefIdentifier {
    std::string className;
    std::string descriptor;
    std::string fieldName;

    std::string toString() const { return descriptor + " " + className + " " + fieldName; }
};

struct FieldInformation {
    std::string descriptor;
    std::string name;
    bool isPrivate = false;
    bool isStatic = false;
};


class ClassFile;
typedef std::shared_ptr<ClassFile> ClassFilePtr;
typedef std::weak_ptr<ClassFile> ClassFileWeakPtr;


class ClassFile {
public:
    static ClassFile parse(BinaryReader& reader);

    void dump(std::ostream&stream) const;

    /** Full qualified name of  the class. */
    const std::string& name() const { return mName; }

    /** Finds the main method. */
    boost::optional<MethodInfo> mainMethod() const;

    /** Find class init method. */
    boost::optional<MethodInfo> clinit() const;


    /** Find a method with given name. */
    boost::optional<MethodInfo> methodWithName(const std::string& name, int requiredFlags = 0) const;

    boost::optional<FieldInfo> fieldWithName(const std::string& name) const;

    /** Finds a method with a given identifier, doesn't look for the class! */
    boost::optional<MethodInfo> methodWithSignature(const MethodIdentifier& identifier) const;

    /** Returns the code block for a method. */
    CodeIdentifier codeForMethod(const MethodInfo& method) const;

    std::string descriptorForMethod(const MethodInfo& method) const {
        return getUtf8Constant(method.descriptorIdx);
    }

    /** Find a class from the constant pool, e.g. for loading. */
    std::string findClass(uint16_t index) const;

    boost::optional<std::string> superClass() const {
        return mHeader.super_class == 0 ? boost::optional<std::string> () : boost::optional<std::string>(findClass(mHeader.super_class));
    }

    std::vector<std::string> interfaces() const {
        std::vector<std::string> result;
        for (auto interfaceId : mInterfaces){
            result.push_back(findClass(interfaceId));
        }
        return result;
    }

    /** Find a method name. */
    std::string methodName(const MethodInfo& method) const {
        return getUtf8Constant(method.nameIdx);
    }

    /** Resolve a method. */
    MethodIdentifier findMethod(uint16_t index) const;

    /** Resolves an interface method. */
    MethodIdentifier findInterfaceMethod(uint16_t index) const;

    /** Resolve a field ref. */
    FieldRefIdentifier findFieldRefIdentifier(uint16_t index) const;

    /** Returns the static fields of this class. */
    std::vector<FieldInformation> fields() const;

    const ConstantEntry & constantEntry(uint16_t index) const {
        return mConstants[index];
    }

    std::string getUtf8Constant(uint16_t idx) const;
    std::string getUtf8Constant(const ConstantEntry& entry) const;

    void setSuperClassFile (const ClassFileWeakPtr& s) { mSuperClassFile = s; }
    ClassFileWeakPtr superClassFile() const { return mSuperClassFile; }

private:

    void parseFromReader(BinaryReader& reader);
    /** Parses an attribute info. Note, it will save bytes into mBytes, but not add
     add the attribute entries to mAttributeEntries, as there are multiple uses of it. */
    AttributeInfo parseAttributeInfo(BinaryReader& reader);
    /** Parses constant entry, but doesn't add to inner tables. */
    ConstantEntry parseConstantEntry(BinaryReader& reader);
    FieldInfo parseFieldInfo(BinaryReader& reader);
    MethodInfo parseMethodInfo(BinaryReader& reader);
    
    ClassFile() {
    }


    ClassFileHeader mHeader = {0};
    std::vector<ConstantEntry> mConstants;
    std::vector<uint16_t> mInterfaces;
    std::vector<FieldInfo> mFieldInfos;
    std::vector<MethodInfo> mMethodInfos;
    std::vector<AttributeInfo> mAttributeEntries;
    
    
    // Contains the non-header bytes
    std::vector<uint8_t> mBytes;
    
    std::string mName;

    /** Index for the UTF8 Word for 'Code'. */
    uint32_t mCodeIndex;
    
    std::string toString(const ConstantEntry& entry) const;
    std::string toString(const MethodInfo& entry) const;
    std::string toString(const AttributeInfo& entry) const;

    ClassFileWeakPtr mSuperClassFile;
};
