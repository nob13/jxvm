#include <iostream>
#include "Interpreter.h"
#include "Ops.h"
#include "ClassFile.h"
#include "DescriptorParser.h"
#include "Variable.h"
#include "MethodOverrides.h"
#include "StringUtils.h"
#include "Log.h"
#include <math.h>

void Frame::ensureLocalArraySpace(int idx){
    assert (idx >= 0);
    if (idx >= localArray.size()){
        logw("Increasing local array size to meet idx ", idx);
        localArray.variables.resize(idx + 1);
    }
}


Interpreter::Interpreter(){
    mMethodOverrides = std::shared_ptr<MethodOverrides>(new MethodOverrides());
    mMethodOverrides->addDefaultOverrides();
    mInstructionCount = 0;
}

void Interpreter::executeFile(const std::string &filename) {
    auto c = mClassLoader.loadByFile(filename);
    executeMain(*c);
}

void Interpreter::executeMain(const ClassFile &clazz) {
    auto main = clazz.mainMethod();
    if (!main.is_initialized()){
        throw std::invalid_argument("No main method found in " + clazz.name());
    }

    logi("Initializing core");
    findInitializedClass("java/nio/charset/StandardCharsets");

    auto systemClass = findInitializedClass("java/lang/System");

    createMainThread();

    auto mainInitializer = systemClass->methodWithName("initializeSystemClass", Flags::STATIC);
    assert((bool)mainInitializer);
    {
        Frame initFrame;
        Variables arguments;
        executeMethod(*systemClass, *mainInitializer, initFrame, arguments);
    }

    findInitializedClass("java/lang/String");

    logi("Executing main method of ", clazz.name());

    Frame initial;
    Variables arguments; // TODO, stdin/stdargs
    // First arg is usually this ptr
    executeMethod(clazz, main.get(), initial, arguments);
}

// Construct a single two-aray function return the same type as introducted
#define MAKE_TRIVIAL_OP(OPCODE,TYPE,ACCESSOR,OP)\
    case OPCODE: { \
        auto v2 = frame.stack.pop(); \
        auto v1 = frame.stack.pop(); \
        assert (v1.type == TYPE); \
        assert (v2.type == TYPE); \
        Variable r (TYPE); \
        r.value.ACCESSOR = v1.value.ACCESSOR OP v2.value.ACCESSOR; \
        frame.stack.push(r); \
        break; \
    }

Variable Interpreter::executeMethod(const ClassFile &clazz, const MethodInfo& method, const Frame &previousFrame,
                                const Variables &arguments) {

    std::string className = clazz.name();
    std::string methodName = clazz.methodName(method);
    std::string descriptor = clazz.descriptorForMethod(method);
    MethodOverrideIdentifier identifier;
    identifier.className = className;
    identifier.methodName = methodName;
    identifier.description = descriptor;

    auto ov = mMethodOverrides->find(identifier);
    if (ov){
        logi("Using override for", identifier.className, identifier.methodName, identifier.description);
        FunctionContext context { this, &mClassLoader, &mMemory, &previousFrame };
        return ov(context, arguments);
    }

    if (method.isNative()){
        logw("Method", className, methodName, descriptor, "is native, skipping");
        DescriptorParser parsedDescriptor (descriptor);
        return Variable(parsedDescriptor.type());
    }


    Frame frame;

    auto code = clazz.codeForMethod(method);
    const ByteRange& bytes = code.code;

    auto pc = bytes.begin;
    for (const Variable & v : arguments.variables){
        frame.localArray.push(v);
    }

    frame.localArray.variables.resize(code.maxLocals);
    if (!(method.accessFlags & Flags::STATIC)){
        assert(!frame.localArray.empty());
        Variable& firstThisArgument = *frame.localArray.variables.begin();
        assert(firstThisArgument.type == ObjectRef);
        assert(firstThisArgument.value.object != nullptr);
        frame.thisp = firstThisArgument.value.object;
    }

    logd("Interpreting", className, methodName, "arg count", arguments.size());
    // std::cout << "  Arguments: ";
    // for (const auto& arg : arguments.variables){
    //    std::cout << arg.toString() << " ";
    // }
    // std::cout << std::endl;

    auto lastPc = pc;
    while (pc < bytes.end){
        mInstructionCount++;
        auto op = *pc;
        auto deltaPc = pc - lastPc;
        // std::cout << "Interpreting: " << className << "::" << methodName << " " << util::toHex((int)op) << " " << ops::opToStr(op) <<  " (dpc=" << deltaPc << ")"  << " instr=" << mInstructionCount << std::endl;
        // std::cout << "  Stack: ";
        // for (auto v : frame.stack.variables){
        //    std::cout << v.toString()/*variableTypeToString(v.type)*/ << " ";
        // }
        // std::cout << std::endl << std::endl;
        lastPc = pc;

        switch (op){
            case ops::nop: pc++; break;
            case ops::dup:
                frame.stack.push(frame.stack.top());
                break;
            case ops::dup_x1:{
                Variable v1 = frame.stack.pop();
                Variable v2 = frame.stack.pop();
                frame.stack.push(v1);
                frame.stack.push(v2);
                frame.stack.push(v1);
                break;
            }
            case ops::dup2:{
                Variable x1 = frame.stack.top();
                if (x1.type == Double || x1.type == Long){
                    // category 2
                    frame.stack.push(x1);
                    break;
                }
                // category 1
                Variable x2 = frame.stack.top(1);
                frame.stack.push(x2);
                frame.stack.push(x1);
                break;
            }
            case ops::iconst_0:
                frame.stack.push((int32_t)0);
                break;
            case ops::iconst_1:
                frame.stack.push((int32_t)1);
                break;
            case ops::iconst_2:
                frame.stack.push((int32_t)2);
                break;
            case ops::iconst_3:
                frame.stack.push((int32_t)3);
                break;
            case ops::iconst_4:
                frame.stack.push((int32_t)4);
                break;
            case ops::iconst_5:
                frame.stack.push((int32_t)5);
                break;
            case ops::iconst_m1:
                frame.stack.push((int32_t)-1);
                break;
            case ops::lconst_0: {
                Variable v(Long);
                v.value.lv = 0;
                frame.stack.push(v);
                break;
            }
            case ops::lconst_1: {
                Variable v(Long);
                v.value.lv = 1;
                frame.stack.push(v);
                break;
            }
            case ops::dconst_0: {
                Variable v(Double);
                v.value.dv = 0.0;
                frame.stack.push(v);
                break;
            }
            case ops::dconst_1: {
                Variable v(Double);
                v.value.dv = 1.0;
                frame.stack.push(v);
                break;
            }
            case ops::sipush: {
                int16_t data = bytes.fetchInt16(pc + 1);
                Variable v(Short);
                v.value.iv = data;
                frame.stack.push(v);
                pc += 2;
                break;
            }
            case ops::aconst_null: {
                Variable v;
                v.type = ObjectRef;
                v.value.object = nullptr;
                frame.stack.push(v);
                break;
            }
            case ops::fconst_0: {
                Variable v;
                v.value.fv = 0.0f;
                v.type = Float;
                frame.stack.push(v);
                break;
            }
            case ops::fconst_1: {
                Variable v;
                v.value.fv = 1.0f;
                v.type = Float;
                frame.stack.push(v);
                break;
            }
            case ops::fconst_2: {
                Variable v;
                v.value.fv = 2.0f;
                v.type = Float;
                frame.stack.push(v);
                break;
            }
            case ops::ldc: {
                uint8_t index = bytes.fetchUint8(pc + 1);
                logd("Loading constant ", index);
                auto constant = clazz.constantEntry(index);
                Variable v;
                switch(constant.tag){
                    case ConstantEntry::FloatTag:
                        v.type = Float;
                        v.value.iv = constant.integerValue();
                        break;
                    case ConstantEntry::IntegerTag:
                        v.type = Integer;
                        v.value.fv = constant.floatValue();
                        break;
                    case ConstantEntry::StringTag: {
                        std::string utf8 = clazz.getUtf8Constant(constant.nameIndex());
                        v = initializeString(utf8, frame);
                        break;
                    }
                    case ConstantEntry::ClassTag: {
                        std::string name = clazz.getUtf8Constant(constant.nameIndex());
                        v = classByName(name);
                        break;
                    }
                    default:
                        throw std::invalid_argument("Unexpected constant type " + std::string(ConstantEntry::tagToString(constant.tag)));
                }
                frame.stack.push(v);
                pc++;
                break;
            }
            case ops::ldc_w: {
                uint16_t index = bytes.fetchUint16(pc + 1);
                logd ("Loading constant ", index);
                auto constant = clazz.constantEntry(index);
                Variable v;
                switch(constant.tag){
                    case ConstantEntry::FloatTag:
                        v.type = Float;
                        v.value.iv = constant.integerValue();
                        break;
                    case ConstantEntry::IntegerTag:
                        v.type = Integer;
                        v.value.fv = constant.floatValue();
                        break;
                    case ConstantEntry::StringTag: {
                        std::string utf8 = clazz.getUtf8Constant(constant.nameIndex());
                        v = initializeString(utf8, frame);
                        break;
                    }
                    case ConstantEntry::ClassTag: {
                        std::string name = clazz.getUtf8Constant(constant.nameIndex());
                        v = classByName(name);
                        break;
                    }
                    default:
                        throw std::invalid_argument("Unexpected constant type " + std::string(ConstantEntry::tagToString(constant.tag)));
                }
                frame.stack.push(v);
                pc+=2;
                break;
            }
            case ops::ldc2_w: {
                uint16_t index = bytes.fetchUint16(pc + 1);
                logd("Loading constant ", index);
                auto constant = clazz.constantEntry(index);
                Variable v;
                switch(constant.tag){
                    case ConstantEntry::DoubleTag:
                        v.type = Double;
                        v.value.dv = constant.doubleValue();
                        break;
                    case ConstantEntry::LongTag:
                        v.type = Long;
                        v.value.lv = constant.longValue();
                        break;
                    default:
                        throw std::invalid_argument("Unexpected constant type " + std::string(ConstantEntry::tagToString(constant.tag)));
                }
                frame.stack.push(v);
                pc+=2;
                break;
            }
            case ops::bipush: {
                int8_t value = bytes.fetchInt8(pc + 1);
                Variable v;
                v.type = Integer;
                v.value.iv = value;
                frame.stack.push(v);
                pc++;
                logd("bipush ", v.value.iv);
                break;
            }
            case ops::bastore: {
                Variable value = frame.stack.pop();
                Variable index = frame.stack.pop();
                Variable arrayRef = frame.stack.pop();
                assert (value.isStoredAsInteger());
                assert (index.isStoredAsInteger());
                assert (arrayRef.isArray());
                int8_t valueToStore = (int8_t) value.value.iv;
                assert (index.value.iv >= 0 && index.value.iv < arrayRef.array()->length);
                arrayRef.array()->values[index.value.iv].iv = valueToStore;
                break;
            }
            case ops::iastore: {
                Variable value = frame.stack.pop();
                Variable index = frame.stack.pop();
                Variable arrayRef = frame.stack.pop();
                assert (value.isStoredAsInteger());
                assert (index.isStoredAsInteger());
                assert (arrayRef.isArray());
                int32_t valueToStore = value.value.iv;
                assert (index.value.iv >= 0 && index.value.iv < arrayRef.array()->length);
                arrayRef.array()->values[index.value.iv].iv = valueToStore;
                break;
            }
            case ops::baload: {
                Variable index = frame.stack.pop();
                Variable arrayRef = frame.stack.pop();
                assert (index.isStoredAsInteger());
                assert (arrayRef.isArray());
                assert (index.value.iv >= 0 && index.value.iv < arrayRef.array()->length);
                int8_t valueToLoad = (int8_t) arrayRef.array()->values[index.value.iv].iv;
                Variable result (Byte);
                result.value.iv = valueToLoad;
                frame.stack.push(result);
                break;
            }
            case ops::newarray: {
                assert(frame.stack.top().isStoredAsInteger());
                assert(frame.stack.top().value.iv >= 0);
                uint32_t length = (uint32_t) frame.stack.pop().value.iv;
                uint8_t valueTypeCode = bytes.fetchUint8(pc + 1);
                VariableType type = Array::fromArrayTypeCode(valueTypeCode);
                Variable array = mMemory.allocateArray(type, length);
                frame.stack.push(array);
                logd("Initialized array of length ", length, " of type ", variableTypeToString(type));
                pc++;
                break;
            }
            case ops::anewarray: {
                uint16_t typeIdx = bytes.fetchUint16(pc + 1);
                const ConstantEntry & entry = clazz.constantEntry(typeIdx);
                assert(entry.tag == ConstantEntry::ClassTag);
                std::string className = clazz.getUtf8Constant(entry.nameIndex());

                Variable len = frame.stack.pop();
                assert(len.isStoredAsInteger());
                assert(len.value.iv >= 0);

                uint32_t length = (uint32_t) len.value.iv;
                VariableType type = ObjectRef;

                Variable array = mMemory.allocateObjectArray(length, className);
                frame.stack.push(array);
                logd("Initialized array of length ", length, " of type ", variableTypeToString(type), "(descriptor=", className, ")");
                pc+=2;
                break;
            }
            case ops::arraylength: {
                Variable top = frame.stack.pop();
                assert(top.isArray());
                uint32_t len = top.array()->length;
                assert (len <= INT_MAX);
                Variable v ((int32_t)len);
                frame.stack.push(v);
                break;
            }
            case ops::castore: {
                assert(frame.stack.size() >= 3);

                assert(frame.stack.top().isStoredAsInteger());
                int32_t v = frame.stack.pop().value.iv;
                uint16_t converted = (uint16_t) (v);

                Variable indexRef = frame.stack.pop();
                Variable arrayRef = frame.stack.pop();

                assert(indexRef.type == Integer);
                assert(arrayRef.isArray());

                arrayRef.array()->values[indexRef.value.iv].iv = converted;
                break;
            }
            case ops::aastore: {
                assert(frame.stack.size() >= 3);

                Variable v = frame.stack.pop();
                Variable indexRef = frame.stack.pop();
                Variable arrayRef = frame.stack.pop();
                assert(indexRef.isStoredAsInteger());
                assert(arrayRef.isArray());
                assert(indexRef.value.iv >= 0 && indexRef.value.iv < arrayRef.array()->length);

                assert(v.memoryType() == arrayRef.array()->memoryType());



                arrayRef.array()->values[indexRef.value.iv].object = v.value.object;
                break;
            }
            case ops::caload: {
                Variable idx = frame.stack.pop();
                Variable array = frame.stack.pop();
                assert (idx.type == Integer);
                assert (array.isArray());
                Variable result (Char);
                result.value.iv = array.array()->values[idx.value.iv].iv;
                frame.stack.push(result);
                break;
            }
            case ops::aaload: {
                Variable idx = frame.stack.pop();
                Variable array = frame.stack.pop();
                assert (idx.type == Integer);
                assert(array.isArray());

                assert (idx.value.iv >= 0 && idx.value.iv < array.array()->length);

                Variable result (array.array()->type);
                result.value.object = array.array()->values[idx.value.iv].object;
                frame.stack.push(result);
                break;
            }
            case ops::new_: {
                auto classIndex = bytes.fetchUint16(pc + 1);
                pc+=2;
                auto className = clazz.findClass(classIndex);

                logi("Allocating class ", className);

                auto classFile = findInitializedClass(className);

                Variable v = mMemory.allocateObject(classFile);
                frame.stack.push(v);
                break;
            }
            case ops::checkcast: {
                auto classIndex = bytes.fetchUint16(pc + 1);
                pc+=2;
                auto className  = clazz.findClass(classIndex);
                Variable var = frame.stack.top();
                if (!className.empty()){
                    if (className[0] == '['){
                        if (var.isArray()){
                            // ok
                            break;
                        }
                        if (var.memoryType() == ObjectRef && var.value.object == nullptr){
                            // ok
                            break;
                        }
                    }
                }
                auto otherClassFile = findInitializedClass(className);

                assert(var.type == ObjectRef);
                if (var.value.object != nullptr){
                    logi("[FixMe] Check cast, slow and wrong");
                    ClassFilePtr current = var.value.object->type;
                    bool proved = false;
                    while (current){
                        if (current == otherClassFile){
                            // yup
                            proved = true;
                            break;
                        }
                        current = current->superClassFile().lock();
                    }
                    if (!proved){
                        logi("FIXME, Could not prove that ", var.value.object->type->name(), " is a sub type of ", className," nothing, works, exceptions are also not supported :/");
                    }
                }
                break;
            }
            case ops::instanceof: {
                auto classIndex = bytes.fetchUint16(pc + 1);
                auto className  = clazz.findClass(classIndex);
                auto otherClassFile = findInitializedClass(className);

                pc+=2;
                Variable var = frame.stack.top();
                assert(var.type == ObjectRef);
                if (var.value.object == nullptr){
                    frame.stack.push(Variable(int32_t(0))); // false
                } else {
                    logi("[FixMe] instanceof, slow and wrong");
                    ClassFilePtr current = var.value.object->type;
                    bool proved = false;
                    while (current && !proved) {
                        auto interfaces = current->interfaces();
                        for (auto interfaceName : interfaces){
                            if (interfaceName == className){
                                proved = true;
                                break;
                            }
                        }
                        if (proved){
                            break;
                        }
                        if (current == otherClassFile) {
                            proved = true;
                            break;
                        }
                        current = current->superClassFile().lock();
                    }
                    if (proved){
                        frame.stack.push(Variable(int32_t(1)));
                    } else {
                        logi("FIXME, Could not prove that ", var.value.object->type->name(),
                        " is a sub type of ", className," nothing, works, exceptions are also not supported :/");
                        frame.stack.push(Variable(int32_t(0)));
                    }
                    logd("Result of instance of ", var.value.object->type->name(), " is ",  className, " -> ",proved);
                }
                break;
            }
            case ops::invokespecial:{
                auto index = bytes.fetchUint16(pc + 1);
                const auto& method = clazz.findMethod(index);

                logd("Invoke special on index ", index, method.toString());
                const auto& methodClazz = findInitializedClass(method.className);
                MethodInfo methodInfo = methodClazz->methodWithSignature(method).get();

                DescriptorParser descriptorParser(method.descriptor);
                auto argCount = descriptorParser.argumentCount();


                Variable result;
                if (methodInfo.accessFlags & Flags::STATIC){
                    Variables args;
                    args.variables = frame.stack.popAndReturnMany(argCount);
                    // omit this pointer?
                    result = executeMethod(clazz, methodInfo, frame, args);
                } else {
                    Variables args;
                    // including this pointer
                    args.variables = frame.stack.popAndReturnMany(argCount + 1);

                    result = executeMethod(*methodClazz, methodInfo, frame, args);
                }
                handleReturn(&frame, result, descriptorParser);
                pc+=2;
                break;
            }
            case ops::invokevirtual:{
                auto index = bytes.fetchUint16(pc + 1);
                pc+=2;
                const auto method = clazz.findMethod(index);

                auto desc = DescriptorParser(method.descriptor);
                Variable thisPointer = *(frame.stack.variables.rbegin() + desc.argumentCount());

                logd("Looking for ", method.methodName, "of", method.className);
                auto methodInfo = virtualMethodDispatch(method, thisPointer);
                logd("Found virtual method ", method.methodName, "of", clazz.name(), "in", methodInfo.first->name());

                // arguments are in same order like on stack, adding argCount and this pointer (which is alredy placed)
                Variables args;
                args.variables = frame.stack.popAndReturnMany(desc.argumentCount() + 1);

                logd("Invoking ", method.toString(), "Arg count", desc.argumentCount());
                Variable result = executeMethod(*methodInfo.first, methodInfo.second, frame, args);
                handleReturn(&frame, result, desc);
                break;
            }
            case ops::invokeinterface: {
                uint16_t index = bytes.fetchUint16(pc + 1);
                uint8_t count = bytes.fetchInt8(pc + 3);

                pc+=4;

                const auto method = clazz.findInterfaceMethod(index);

                auto desc = DescriptorParser(method.descriptor);
                Variable thisPointer = *(frame.stack.variables.rbegin() + desc.argumentCount());

                auto methodInfo = virtualMethodDispatch(method, thisPointer);
                logd("Found virtual method ", method.methodName, "of", clazz.name(), "in", methodInfo.first->name());

                // arguments are in same order like on stack, adding argCount and this pointer (which is alredy placed)
                Variables args;
                args.variables = frame.stack.popAndReturnMany(desc.argumentCount() + 1);

                logd("Invoking ", method.toString(), " Arg count ", desc.argumentCount());
                Variable result = executeMethod(*methodInfo.first, methodInfo.second, frame, args);
                handleReturn(&frame, result, desc);
                break;
            }
            // aload (object references)
            case ops::aload_0:{
                Variable v = frame.localArray.variables[0];
                frame.stack.push(v);
                break;
            }
            case ops::aload_1:{
                frame.stack.push(frame.localArray.variables[1]);
                break;
            }
            case ops::aload_2:{
                frame.stack.push(frame.localArray.variables[2]);
                break;
            }
            case ops::aload_3:{
                frame.stack.push(frame.localArray.variables[3]);
                break;
            }

            // iload (integers)
            case ops::iload_0:
            case ops::lload_0:
            case ops::fload_0:
            case ops::dload_0:
            {
                frame.stack.push(frame.localArray.variables[0]);
                break;
            }
            case ops::iload_1:
            case ops::fload_1:
            case ops::lload_1:
            case ops::dload_1:{
                frame.stack.push(frame.localArray.variables[1]);
                break;
            }
            case ops::iload_2:
            case ops::fload_2:
            case ops::dload_2:
            case ops::lload_2:{
                frame.stack.push(frame.localArray.variables[2]);
                break;
            }
            case ops::iload_3:
            case ops::lload_3:
            case ops::fload_3:
            case ops::dload_3:{
                frame.stack.push(frame.localArray.variables[3]);
                break;
            }
            case ops::istore_0:
            case ops::lstore_0:
            case ops::fstore_0:
            case ops::dstore_0:
            case ops::astore_0: {
                Variable top = frame.stack.pop();
                frame.ensureLocalArraySpace(0); // necessary?

                frame.localArray.variables[0] = top;
                break;
            }
            case ops::istore_1:
            case ops::astore_1:
            case ops::lstore_1:
            case ops::fstore_1:
            case ops::dstore_1:{
                Variable top = frame.stack.pop();
                frame.ensureLocalArraySpace(1); // necessary?

                frame.localArray.variables[1] = top;
                break;
            }
            case ops::istore_2:
            case ops::astore_2:
            case ops::lstore_2:
            case ops::dstore_2:
            case ops::fstore_2:{
                Variable top = frame.stack.pop();
                frame.ensureLocalArraySpace(2); // necessary?

                frame.localArray.variables[2] = top;
                break;
            }
            case ops::istore_3:
            case ops::astore_3:
            case ops::lstore_3:
            case ops::dstore_3:
            case ops::fstore_3:{
                Variable top = frame.stack.pop();
                frame.ensureLocalArraySpace(3); // necessary?

                frame.localArray.variables[3] = top;
                break;
            }
            case ops::astore:
            case ops::istore:
            case ops::dstore:
            case ops::lstore:
            case ops::fstore:{
                uint8_t idx = bytes.fetchUint8(pc + 1);
                pc += 1;
                Variable v = frame.stack.pop();

                // assert(v.type == Integer);
                frame.ensureLocalArraySpace(idx);
                frame.localArray.variables[idx] = v;
                break;
            }
            case ops::aload:
            case ops::lload:
            case ops::iload:
            case ops::fload:
            case ops::dload:{
                uint8_t idx = bytes.fetchUint8(pc + 1);
                pc += 1;
                frame.ensureLocalArraySpace(idx);

                Variable v = frame.localArray.variables[idx];
                if (v.type == None && op == ops::aload){
                    v = Variable(ObjectRef); // workaround
                }

                // assert(v.type == Integer);
                frame.stack.push(v);
                break;
            }
            case ops::return_:
                return Variable();
            case ops::ireturn: {
                assert(!frame.stack.empty());
                auto ir = frame.stack.pop();
                assert(ir.isStoredAsInteger());
                return ir;
            }
            case ops::dreturn: {
                assert(!frame.stack.empty());
                auto dr = frame.stack.pop();
                assert(dr.type == Double);
                return dr;
            }
            case ops::freturn: {
                assert(!frame.stack.empty());
                auto fr = frame.stack.pop();
                assert(fr.type == Float);
                return fr;
            }
            case ops::lreturn: {
                assert(!frame.stack.empty());
                auto lr = frame.stack.pop();
                assert(lr.type == Long);
                return lr;
            }
            case ops::areturn: {
                assert(!frame.stack.empty());
                auto objectReturn = frame.stack.pop();
                assert(objectReturn.type == ObjectRef || objectReturn.type == ArrayRef);
                return objectReturn;
            }
            case ops::iadd: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                frame.stack.push(Variable(v1.value.iv + v2.value.iv));
                break;
            }
            case ops::isub: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                frame.stack.push(Variable(v1.value.iv - v2.value.iv));
                break;
            }
            case ops::iand: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                frame.stack.push(Variable(v1.value.iv & v2.value.iv));
                break;
            }
            case ops::ior: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                frame.stack.push(Variable(v1.value.iv | v2.value.iv));
                break;
            }
            case ops::irem: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                assert(v2.type == Integer);
                assert(v1.type == Integer);
                int32_t result = v1.value.iv % v2.value.iv;
                frame.stack.push(Variable(result));
                break;
            }
            case ops::iinc: {
                uint8_t idx = bytes.fetchUint8(pc + 1);
                int8_t cnst = bytes.fetchInt8(pc + 2);
                pc+=2;
                frame.ensureLocalArraySpace(idx);
                Variable & local = frame.localArray.variables[idx];
                assert(local.type == Integer);
                local.value.iv+=cnst;
                break;
            }
            case ops::i2l: {
                auto v1 = frame.stack.pop();
                assert(v1.isStoredAsInteger());
                Variable r (Long);
                r.value.lv = v1.value.iv;
                frame.stack.push(r);
                break;
            }
            case ops::i2d: {
                auto v1 = frame.stack.pop();
                assert(v1.isStoredAsInteger());
                Variable r (Double);
                r.value.dv = (double) r.value.iv;
                frame.stack.push(r);
                break;
            }
            case ops::i2c: {
                auto v1 = frame.stack.pop();
                assert(v1.isStoredAsInteger());
                Variable r (Char);
                r.value.lv = v1.value.iv;
                frame.stack.push(r);
                break;
            }
            case ops::i2b: {
                auto v1 = frame.stack.pop();
                assert(v1.isStoredAsInteger());
                Variable r (Byte);
                r.value.iv = (int8_t) v1.value.iv;
                frame.stack.push(r);
                break;
            }
            case ops::i2f: {
                auto v1 = frame.stack.pop();
                assert(v1.isStoredAsInteger());
                Variable r (Float);
                r.value.fv = v1.value.iv;
                frame.stack.push(r);
                break;
            }
            case ops::f2i: {
                auto v1 = frame.stack.pop();
                assert(v1.type == Float);
                Variable r (Integer);
                r.value.iv = (int32_t)v1.value.fv;
                frame.stack.push(r);
                break;
            }
            case ops::f2d: {
                auto v1 = frame.stack.pop();
                assert(v1.type == Float);
                Variable r (Double);
                r.value.dv = (double)v1.value.fv;
                frame.stack.push(r);
                break;
            }
            case ops::f2l: {
                auto v1 = frame.stack.pop();
                assert(v1.type == Float);
                Variable r (Long);
                r.value.lv = (int64_t)v1.value.fv;
                frame.stack.push(r);
                break;
            }
            case ops::l2d: {
                auto v1 = frame.stack.pop();
                assert(v1.type == Long);
                Variable r (Double);
                r.value.dv = (double)v1.value.lv;
                frame.stack.push(r);
                break;
            }
            case ops::l2i: {
                auto v1 = frame.stack.pop();
                assert(v1.type == Long);
                Variable r (Integer);
                r.value.iv = (int32_t)v1.value.lv;
                frame.stack.push(r);
                break;
            }
            case ops::d2l: {
                auto v1 = frame.stack.pop();
                assert(v1.type == Double);
                Variable r (Long);
                r.value.lv = (int64_t)v1.value.dv;
                frame.stack.push(r);
                break;
            }
            case ops::d2i: {
                auto v1 = frame.stack.pop();
                assert(v1.type == Double);
                Variable r (Integer);
                r.value.iv = (int32_t)v1.value.dv;
                frame.stack.push(r);
                break;
            }
            case ops::d2f: {
                auto v1 = frame.stack.pop();
                assert(v1.type == Double);
                Variable r (Float);
                r.value.fv = (float)v1.value.dv;
                frame.stack.push(r);
                break;
            }
            case ops::imul: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                assert (v1.isStoredAsInteger());
                assert (v2.isStoredAsInteger());
                Variable r (Integer);
                r.value.iv = v1.value.iv * v2.value.iv;
                frame.stack.push(r);
                break;
            }
            case ops::idiv: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                assert (v1.isStoredAsInteger());
                assert (v2.isStoredAsInteger());
                Variable r (Integer);
                r.value.iv = v1.value.iv / v2.value.iv;
                frame.stack.push(r);
                break;
            }
            case ops::ineg: {
                auto v1 = frame.stack.pop();
                assert (v1.isStoredAsInteger());
                Variable r (Integer);
                r.value.iv = -1 * v1.value.iv;
                frame.stack.push(r);
                break;
            }
            case ops::lshl: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                assert (v1.type == Long);
                assert (v2.type == Integer);
                Variable r (Long);
                r.value.lv = v1.value.lv << (v2.value.iv & 0x3f);
                frame.stack.push(r);
                break;
            }
            case ops::lshr: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                assert (v1.type == Long);
                assert (v2.type == Integer);
                Variable r (Long);
                r.value.lv = v1.value.lv >> (v2.value.iv & 0x3f);
                frame.stack.push(r);
                break;
            }
            case ops::iushr: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                assert (v1.type == Integer);
                assert (v2.type == Integer);
                Variable r (Integer);
                uint32_t result = (uint32_t)(v1.value.iv) >> (uint32_t)(v2.value.iv & 0x3f);
                r.value.iv = (int32_t) result;
                frame.stack.push(r);
                break;
            }
            case ops::ishr: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                assert (v1.type == Integer);
                assert (v2.type == Integer);
                Variable r (Integer);
                int32_t result = v1.value.iv >> (v2.value.iv & 0x3f);
                r.value.iv = result;
                frame.stack.push(r);
                break;
            }
            case ops::ishl: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                assert (v1.type == Integer);
                assert (v2.type == Integer);
                Variable r (Integer);
                int32_t result = v1.value.iv << (v2.value.iv & 0x3f);
                r.value.iv = result;
                frame.stack.push(r);
                break;
            }
            case ops::ixor: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                assert (v1.type == Integer);
                assert (v2.type == Integer);
                Variable r (v1.value.iv ^ v2.value.iv);
                frame.stack.push(r);
                break;
            }
            case ops::land: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                assert (v1.type == Long);
                assert (v2.type == Long);
                Variable r (Long);
                r.value.lv = v1.value.lv & v2.value.lv;
                frame.stack.push(r);
                break;
            }
            MAKE_TRIVIAL_OP(ops::ladd, Long, lv, +)
            MAKE_TRIVIAL_OP(ops::lsub, Long, lv, -)
            MAKE_TRIVIAL_OP(ops::lmul, Long, lv, *)
            MAKE_TRIVIAL_OP(ops::ldiv, Long, lv, /)
            MAKE_TRIVIAL_OP(ops::lrem, Long, lv, %)

            case ops::lneg: {
                auto v1 = frame.stack.pop();
                assert (v1.type == Long);
                Variable r (Long);
                r.value.lv = -1L * v1.value.lv;
                frame.stack.push(r);
                break;
            }

            MAKE_TRIVIAL_OP(ops::fadd, Float, fv, +)
            MAKE_TRIVIAL_OP(ops::fsub, Float, fv, -)
            MAKE_TRIVIAL_OP(ops::fmul, Float, fv, *)
            MAKE_TRIVIAL_OP(ops::fdiv, Float, fv, /)

            case ops::frem: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                assert (v1.type == Float);
                assert (v2.type == Float);
                Variable r (Float);
                r.value.fv = fmod(v1.value.fv, v2.value.fv);
                frame.stack.push(r);
                break;
            }

            case ops::fneg: {
                auto v1 = frame.stack.pop();
                assert (v1.type == Float);
                Variable r (Float);
                r.value.fv = -1.0f * v1.value.fv;
                frame.stack.push(r);
                break;
            }

            MAKE_TRIVIAL_OP(ops::dadd, Double, dv, +)
            MAKE_TRIVIAL_OP(ops::dsub, Double, dv, -)
            MAKE_TRIVIAL_OP(ops::ddiv, Double, dv, /)
            MAKE_TRIVIAL_OP(ops::dmul, Double, dv, *)

            case ops::drem: {
                auto v2 = frame.stack.pop();
                auto v1 = frame.stack.pop();
                assert (v1.type == Double);
                assert (v2.type == Double);
                Variable r (Double);
                r.value.dv = fmod(v1.value.dv, v2.value.dv);
                frame.stack.push(r);
                break;
            }

            case ops::dneg: {
                auto v1 = frame.stack.pop();
                assert (v1.type == Double);
                Variable r (Double);
                r.value.dv = -1.0 * v1.value.dv;
                frame.stack.push(r);
                break;
            }

            case ops::pop: {
                frame.stack.pop();
                break;
            }
            case ops::getstatic: {
                auto index = bytes.fetchUint16(pc + 1);
                auto info = clazz.findFieldRefIdentifier(index);
                pc+=2;

                logd("Get static, index ", index, " info ", info.toString());
                findInitializedClass(info.className);

                Variable v = mMemory.getGlobal(info.className, info.fieldName);
                frame.stack.push(v);
                break;
            }
            case ops::putstatic: {
                auto index = bytes.fetchUint16(pc + 1);
                auto info = clazz.findFieldRefIdentifier(index);
                pc+=2;

                logd("Put static, index " index, " info ", info.toString());
                auto otherClazz = findInitializedClass(info.className);

                mMemory.putGlobal(info.className, info.fieldName, frame.stack.pop());
                break;
            }
            case ops::putfield: {
                uint16_t fieldId = bytes.fetchUint16(pc + 1);
                FieldRefIdentifier fieldRefIdentifier = clazz.findFieldRefIdentifier(fieldId);
                std::string fieldName = /*fieldRefIdentifier.className + "__" +*/ fieldRefIdentifier.fieldName;

                Variable v = frame.stack.pop();
                Variable objectRef = frame.stack.pop();
                assert(objectRef.type == ObjectRef);

                // std::string fieldName = clazz.getFieldName(fieldId);
                logd("Put field ", fieldId, " current class ", clazz.name(), " name: ", fieldName);

                assert(objectRef.value.object != nullptr);
                Variable * field = objectRef.value.object->publicField(fieldRefIdentifier.fieldName);
                if (field == nullptr){
                    // TODO: Check!!
                    field = objectRef.value.object->privateField(fieldRefIdentifier.className, fieldRefIdentifier.fieldName);
                }
                if (field == nullptr){
                    throw std::invalid_argument("Field " + fieldName + " not found in instance of " + frame.thisp->type->name());
                }
                *field = v;
                pc+=2;
                break;
            }
            case ops::getfield: {
                uint16_t fieldId = bytes.fetchUint16(pc + 1);

                Variable objectRef = frame.stack.pop();
                assert(objectRef.type == ObjectRef);

                FieldRefIdentifier fieldRefIdentifier = clazz.findFieldRefIdentifier(fieldId);
                std::string fieldName = /*fieldRefIdentifier.className + "__" +*/ fieldRefIdentifier.fieldName;

                Variable * field = objectRef.value.object->publicField(fieldRefIdentifier.fieldName);
                if (field == nullptr){
                    // TODO: Check!!
                    field = objectRef.value.object->privateField(fieldRefIdentifier.className, fieldRefIdentifier.fieldName);
                }
                if (field == nullptr){
                    logi ("Valid Fields ", objectRef.value.object->fields.size());
                    logi ("Object type ", objectRef.value.object->type->name());
                    for (auto field : objectRef.value.object->fields){
                        logi("Field ", field.first);
                    }
                    throw std::invalid_argument("Could not find field " + fieldName);
                }
                frame.stack.push(*field);
                logd("Loaded field ", fieldName,  "type", variableTypeToString(field->type));
                pc+=2;
                break;
            }
            case ops::invokestatic: {
                auto index = bytes.fetchUint16(pc + 1);
                auto info = clazz.findMethod(index);
                logd("InvokeStatic, index ", index, "info", info.toString());
                auto clazz = findInitializedClass(info.className);
                auto methodInfo = clazz->methodWithSignature(info).get();

                DescriptorParser descriptorParser(info.descriptor);

                auto argCount = descriptorParser.argumentCount();
                // arguments are in same order like on stack, adding argCount
                Variables args;
                args.variables = frame.stack.popAndReturnMany(argCount);

                Variable result = executeMethod(*clazz, methodInfo, frame, args);
                handleReturn(&frame, result, descriptorParser);

                pc+=2;
                break;
            }
            case ops::iflt:
            case ops::ifge: {
                int16_t target = bytes.fetchInt16(pc + 1);
                Variable top = frame.stack.pop();
                assert(top.isStoredAsInteger());
                bool isGe = op == ops::ifge;
                if ((top.value.iv >= 0) == isGe){
                    logd("ifge/lt Jumping! ", isGe, top.value.iv);
                    pc = pc + target;
                    continue;
                } else {
                    logd("No jump");
                }
                pc+=2;
                break;
            }
            case ops::lcmp: {
                Variable v2 = frame.stack.pop();
                Variable v1 = frame.stack.pop();
                assert (v2.type == Long);
                assert (v1.type == Long);
                int32_t result = v1.value.lv == v2.value.lv ? 0 : ( v1.value.lv > v2.value.lv ? 1 : -1);
                frame.stack.push(result);
                break;
            }
            case ops::if_icmplt:
            case ops::if_icmpge: {
                Variable v2 = frame.stack.pop();
                Variable v1 = frame.stack.pop();
                int16_t target = bytes.fetchInt16(pc + 1);
                assert(v2.isStoredAsInteger());
                assert(v1.isStoredAsInteger());
                bool isGt = op == ops::if_icmpge;
                if ((v1.value.iv >= v2.value.iv) == isGt){
                    logd("if_cmpge/if_icmplt Jumping!");
                    pc = pc + target;
                    continue;
                } else {
                    logd("No jump ");
                }
                pc+=2;
                break;
            }
            case ops::if_icmpne:
            case ops::if_icmpeq: {
                bool isEq = op == ops::if_icmpeq;

                Variable v2 = frame.stack.pop();
                Variable v1 = frame.stack.pop();
                int16_t target = bytes.fetchInt16(pc + 1);

                assert(v2.isStoredAsInteger());
                assert(v1.isStoredAsInteger());
                if ((v1.value.iv == v2.value.iv) == isEq){
                    logd("if_icmpne/if_icmpeq Jumping! ");
                    pc = pc + target;
                    continue;
                } else {
                    logd("No jump ");
                }
                pc+=2;
                break;
            }
            case ops::ifgt:
            case ops::ifle: {
                int16_t target = bytes.fetchInt16(pc + 1);
                Variable top = frame.stack.pop();
                assert(top.isStoredAsInteger());
                bool isGt = op == ops::ifgt;
                if (top.value.iv > 0 == isGt){
                    logd("ifle/ifgt Jumping! ");
                    pc = pc + target;
                    continue;
                } else {
                    logd("No jump ", top.value.iv, " isGt ",isGt);
                }
                pc+=2;
                break;
            }
            case ops::ifnull:
            case ops::ifnonnull: {
                int16_t target = bytes.fetchInt16(pc + 1);
                Variable top = frame.stack.pop();
                assert(top.type == ObjectRef || top.type == ArrayRef);
                bool isIfNull = op == ops::ifnull;
                if ((top.value.object == nullptr) == isIfNull){
                    logd("ifnull Jumping! isIfNull", isIfNull, top.value.object);
                    pc = pc + target;
                    continue;
                } else {
                    logd("No jump");
                }
                pc+=2;
                break;
            }
            case ops::if_acmpeq:
            case ops::if_acmpne: {
                int16_t target = bytes.fetchInt16(pc + 1);
                Variable v2 = frame.stack.pop();
                Variable v1 = frame.stack.pop();
                bool isEq = op == ops::if_acmpeq;
                assert (v2.type == ObjectRef);
                assert (v1.type == ObjectRef);
                if ((v1.value.object == v2.value.object) == isEq){
                    logd("if_acmpne Jumping! isEq", isEq);
                    pc = pc + target;
                    continue;
                } else {
                    logd("No jump");
                }
                pc+=2;
                break;
            }
            case ops::ifeq:
            case ops::ifne: {
                int16_t target = bytes.fetchInt16(pc + 1);
                Variable top = frame.stack.pop();
                assert(top.type == Integer || top.type == Boolean);
                bool isEq = op == ops::ifeq;
                if ((top.value.iv == 0) == isEq){
                    logd("ifne/ifeq Jumping! ", top.value.iv);
                    pc = pc + target;
                    continue;
                } else {
                    logd("No jump ", top.value.iv,  "isEq=", isEq);
                }
                pc+=2;
                break;
            }
            case ops::if_icmple:
            case ops::if_icmpgt: {
                int16_t target = bytes.fetchInt16(pc + 1);
                Variable b = frame.stack.pop();
                Variable a = frame.stack.pop();

                bool isGt = op == ops::if_icmpgt;
                assert(a.isStoredAsInteger());
                assert(b.isStoredAsInteger());
                if ((a.value.iv > b.value.iv) == isGt){
                    logd("if_icmple/if_icmpgt Jumping! isGt=", isGt);
                    pc = pc + target;
                    continue;
                } else {
                    logd("No jump");
                }
                pc+=2;
                break;
            }
            case ops::goto_: {
                int16_t target = bytes.fetchInt16(pc + 1);
                pc = pc + target;
                continue;
            }
            case ops::fcmpl:
            case ops::fcmpg: {
                Variable b = frame.stack.pop();
                Variable a = frame.stack.pop();
                assert (a.type == Float);
                assert (b.type == Float);
                Variable result;
                result.type = Integer;
                result.value.iv = (a.value.fv == b.value.fv) ? 0 : ( a.value.fv > b.value.fv ? 1 : -1);
                bool isFcmpg = op == ops::fcmpg;
                if (a.value.fv == NAN || b.value.fv == NAN){
                    result.value.iv = isFcmpg ? 1 : -1;
                }
                frame.stack.push(result);
                break;
            }
            case ops::dcmpg:
            case ops::dcmpl: {
                bool isDcmpg = op == ops::dcmpg;
                Variable b = frame.stack.pop();
                Variable a = frame.stack.pop();
                assert (a.type == Double);
                assert (b.type == Double);
                Variable result;
                result.type = Integer;
                result.value.iv = (a.value.fv == b.value.fv) ? 0 : (a.value.fv > b.value.fv ? 1 : -1);
                if (a.value.fv == NAN || b.value.fv == NAN) {
                    result.value.iv = isDcmpg ? 1 : -1;
                }
                frame.stack.push(result);
                break;
            }
            case ops::monitorenter:
                logd("TODO: Monitor enter not supported");
                frame.stack.pop();
            break;
            case ops::monitorexit:
                logd("TODO: Monitor exit not supported");
                frame.stack.pop();
            break;
            case ops::athrow: {
                Variable exceptionObject = frame.stack.pop();
                assert(exceptionObject.type == ObjectRef);
                JvmException exception(exceptionObject);
                throw exception;
                break;
            }
            case ops::lookupswitch: {
                auto baseAddress = pc;

                Variable key = frame.stack.pop();
                assert(key.isStoredAsInteger());
                // Step 1 find out padding
                pc ++; // go away from current instruction
                ssize_t currentDelta = pc - bytes.begin;
                size_t pad = (4 - currentDelta % 4) % 4;
                logd("Lookupswitch, pad = ", pad);
                pc += pad;
                ssize_t afterDelta = pc - bytes.begin;
                assert(afterDelta % 4 == 0);

                int32_t defaultValue = bytes.fetchInt32(pc);
                int32_t nPairs = bytes.fetchInt32(pc + 4);
                assert (nPairs >= 0);

                pc += 8;
                logd("Number of pairs ", nPairs);
                bool found = false;
                for (int32_t i = 0 ; i < nPairs; i++){
                    int32_t v = bytes.fetchInt32(pc);
                    int32_t jumpAddressOffset = bytes.fetchInt32(pc + 4);
                    if (v == key.value.iv){
                        logd("lookupswitch jump at ", v);
                        pc = baseAddress + jumpAddressOffset;
                        found = true;
                        break;
                    }
                    pc+=8;
                }
                if (found){
                    continue;
                }
                logd("Not found, jump using default delta");
                pc = baseAddress + defaultValue;
                continue;
                break;
            }
            default:
                throw std::invalid_argument(std::string("Unsupported file, opcode=") + ops::opToStr(op));
        }


        pc++;
    }
    logi("Leaving... ");
    return Variable();
}

Variable Interpreter::callStatic(const std::string &className, const std::string &methodName,
                                 const Variables &arguments) {
    auto clazz = findInitializedClass(className);
    MethodInfo method = clazz->methodWithName(methodName, 0).get();
    return executeMethod(*clazz, method, Frame(), arguments);
}

Variable Interpreter::classByName(const std::string& name){
    Variable className = initializeString(name, Frame());

    logd("classByName ", name);
    auto clazz = findInitializedClass("java/lang/Class");
    Variable result = mMemory.allocateObject(clazz);

    result.value.object->fields["__name"] = className;

    return result;
}

ClassFilePtr Interpreter::findInitializedClass(const std::string &name) {
    auto result = classLoader().loadByName(name);
    if (mInitializedClasses.count(name) > 0){
        return result;
    }
    auto superClass = result->superClass();
    if (superClass){
        findInitializedClass(*superClass);
    }

    prepareClazz(result);
    initClass(result);
    return result;
}

void Interpreter::handleReturn(Frame *frame, Variable returnValue, const DescriptorParser &methodSignature) {
    VariableType expectedType = methodSignature.type();
    if (returnMemoryType(expectedType) != returnMemoryType(returnValue.type)){
        throw std::invalid_argument(std::string("Expected return type ") + variableTypeToString(returnMemoryType(expectedType)) + " got " + variableTypeToString(returnMemoryType(returnValue.type)));
    }
    if (expectedType != None){
        frame->stack.push(returnValue);
    }
}

void Interpreter::prepareClazz(const ClassFilePtr &clazz) {
    auto fields = clazz->fields();
    for (const auto& field: fields){
        if (!field.isStatic){
            continue;
        }
        logd("Initializing static field ", field.name);
        DescriptorParser parser(field.descriptor);
        if (parser.isMethod()){
            throw std::invalid_argument("Expected a field, not a method for " + clazz->name() + "/" + field.name + " description: " + field.descriptor);
        }
        mMemory.initGlobal(clazz->name(), field.name, parser.type());
    }
    logd("Prepared ", fields.size(), " fields for ", clazz->name());
}

void Interpreter::initClass(const ClassFilePtr &clazz) {
    // Adding it before, even if it's not yet initialized in order to avoid initializing loops.
    mInitializedClasses.insert(clazz->name());
    // Run static initializer
    auto initializer = clazz->clinit();
    if (initializer){
        Frame nullFrame;
        Variables nullArguments;
        logd("Executing initializer of ", clazz->name());
        executeMethod(*clazz, initializer.get(), nullFrame, nullArguments);
    }
}

std::pair<ClassFilePtr, MethodInfo> Interpreter::virtualMethodDispatch(const MethodIdentifier &method, const Variable &thisPointer) {
    assert(thisPointer.type == ObjectRef);
    ClassFilePtr current = thisPointer.value.object->type;
    while (current){
        boost::optional<MethodInfo> info = current->methodWithSignature(method);
        if (info){
            return std::make_pair(current, info.get());
        }
        boost::optional<std::string> superClass = current->superClass();
        if (!superClass){
            break;
        }
        current = findInitializedClass(superClass.get());
    }
    throw std::invalid_argument("Could not find method " + method.className + "/" + method.methodName + " in " + thisPointer.value.object->type->name());
}

Variable Interpreter::initializeString(const std::string &content, const Frame& previousFrame) {
    logd("Initializing string ", content);

    std::u16string utf16 = stringutils::utf8ToUtf16(content);

    auto stringClass = findInitializedClass("java/lang/String");
    Variable str = mMemory.allocateObject(stringClass);

    auto charArray = mMemory.allocateArray(Char, utf16.length());


    for (size_t i = 0; i < utf16.length(); i++){
        charArray.array()->values[i].iv  = utf16[i];
    }



    MethodIdentifier identifier;
    identifier.className = "jx/lang/String";
    identifier.descriptor = "([C)V";
    identifier.methodName = "<init>";


    MethodInfo info = stringClass->methodWithSignature(identifier).get();

    Variables args;
    args.push(str);
    args.push(charArray);

    executeMethod(*stringClass, info, previousFrame, args);
    return str;
}

void Interpreter::createMainThread() {
    auto threadClass = findInitializedClass("java/lang/Thread");
    auto threadGroupClass = findInitializedClass("java/lang/ThreadGroup");
    Variable threadGroup = mMemory.allocateObject(threadGroupClass);
    MethodInfo threadGroupInit = threadGroupClass->methodWithName("<init>").get();
    Variables initArgs;
    initArgs.push(threadGroup);
    executeMethod(*threadGroupClass, threadGroupInit, Frame(), initArgs);

    mMainThread = mMemory.allocateObject(threadClass);
    // hack!
    Variable * prioField = mMainThread.value.object->privateField("java/lang/Thread", "priority");
    assert(prioField);
    prioField->value.iv = 5;


    Variable mainThreadName = initializeString("main", Frame());

    MethodIdentifier identifier;
    identifier.methodName = "<init>";
    identifier.descriptor = "(Ljava/lang/ThreadGroup;Ljava/lang/String;)V";
    MethodInfo threadInitMethod = threadClass->methodWithSignature(identifier).get();
    Variables threadInitArgs;
    threadInitArgs.push(mMainThread);
    threadInitArgs.push(threadGroup);
    threadInitArgs.push(mainThreadName);

    executeMethod(*threadClass, threadInitMethod, Frame(), threadInitArgs);
}

