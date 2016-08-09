#include "MethodOverrides.h"
#include "Variable.h"
#include "StringUtils.h"
#include "Log.h"

static Variable doPrivilegedFake(const FunctionContext& context, const Variables& variables){
    assert(variables.size() == 1);
    Variable privilegedAction = variables.variables[0];
    assert(privilegedAction.type == ObjectRef);

    auto method = privilegedAction.value.object->type->methodWithName("run");
    assert(method);
    Variables arguments;
    arguments.push(privilegedAction); // this pointer
    Variable result = context.interpreter->executeMethod(*privilegedAction.value.object->type, method.get(), *context.previousFrame, arguments);
    return result;
}

void MethodOverrides::addDefaultOverrides(){
    add("java/lang/Double", "longBitsToDouble", "(J)D", [](const FunctionContext&, const Variables& variables){
        assert (variables.size() == 1);
        const Variable & arg0 = variables.variables[0];
        assert (arg0.type == Long);
        double d = reinterpret_cast<const double&> (arg0.value.lv);
        Variable result (Double);
        result.value.dv = d;
        return result;
    });
    add("java/lang/Object", "hashCode", "()I", [](const FunctionContext&, const Variables& variables){
        assert (variables.size() == 1);
        auto thisp = variables.variables[0].value.object;
        return Variable(reinterpret_cast<int32_t&>(thisp));
    });
    add("java/lang/String", "intern", "()Ljava/lang/String;", [](const FunctionContext&, const Variables& variables){
        assert (variables.size() == 1);
        logw("Fake implementation of String.intern!!");
        return variables.variables[0];
    });
    add("java/lang/System", "arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V", [](const FunctionContext& context, const Variables& variables){
        assert (variables.size() == 5);
        Variable src = variables.variables[0];
        Variable srcPos = variables.variables[1];
        Variable target = variables.variables[2];
        Variable targetPos = variables.variables[3];
        Variable length = variables.variables[4];
        assert (src.isArray());
        assert (target.isArray());
        assert(srcPos.isStoredAsInteger());
        assert(targetPos.isStoredAsInteger());
        assert(length.isStoredAsInteger());
        assert(targetPos.value.iv + length.value.iv <= target.value.object->array->length);
        assert(srcPos.value.iv + length.value.iv <= src.value.object->array->length);
        for (int32_t i = 0; i < length.value.iv; i++){
            target.value.object->array->values[targetPos.value.iv + i] =src.value.object->array->values[srcPos.value.iv + i];
        }
        return Variable();
    });
    add("java/lang/System", "initProperties", "(Ljava/util/Properties;)Ljava/util/Properties;", [](const FunctionContext& context, const Variables& variables){
        assert (variables.size() == 1);
        return context.interpreter->callStatic("jx/system/System", "initSystemProperties", variables);
    });


    add("sun/reflect/Reflection", "getCallerClass", "()Ljava/lang/Class;", [](const FunctionContext& context, const Variables& variables){
        assert(variables.size() == 0);
        // TODO: Name
        auto classClassFile = context.loader->loadByName("java/lang/Class");
        auto instance = context.memory->allocateObject(classClassFile);
        return instance;
    });
    add("java/security/AccessController", "doPrivileged", "(Ljava/security/PrivilegedAction;)Ljava/lang/Object;", doPrivilegedFake);
    add("java/security/AccessController", "doPrivileged", "(Ljava/security/PrivilegedExceptionAction;)Ljava/lang/Object;", doPrivilegedFake);
    add("java/lang/Thread", "currentThread", "()Ljava/lang/Thread;", [](const FunctionContext& context, const Variables& variables){
        assert(variables.size() == 0);
        return context.interpreter->mainThread();
    });
    // hack (Ljava/lang/Class;Ljava/lang/Class;Ljava/lang/String;)Ljava/util/concurrent/atomic/AtomicReferenceFieldUpdater; jx/util/concurrent/atomic/AtomicReferenceFieldUpdater newUpdater
    add("java/util/concurrent/atomic/AtomicReferenceFieldUpdater", "newUpdater", "(Ljava/lang/Class;Ljava/lang/Class;Ljava/lang/String;)Ljava/util/concurrent/atomic/AtomicReferenceFieldUpdater;",[](const FunctionContext& context, const Variables& variables){
        assert(variables.size() == 3);
        Variable result(ObjectRef);
        // implementation can't work in the moment due missing reflection
        return result;
    });
    // disabling extended charsets, otherwise it crashes because there is no support for newInstance()
    add("java/nio/charset/Charset$ExtendedProviderHolder", "extendedProvider", "()Ljava/nio/charset/spi/CharsetProvider;", [](const FunctionContext& context, const Variables& variables) {
        // Returning nullptr, no additonal Charsets available
        assert(variables.size() == 0);
        Variable result(ObjectRef);
        return result;
    });
    add("java/lang/Float", "floatToRawIntBits", "(F)I", [](const FunctionContext& context, const Variables& variables) {
        // Returning nullptr, no additonal Charsets available
        assert(variables.size() == 1);
        auto f = variables.variables[0];
        assert(f.type == Float);
        return Variable(reinterpret_cast<int32_t&> (f.value.fv));
    });
    add("java/lang/Double", "doubleToRawLongBits", "(D)J", [](const FunctionContext& context, const Variables& variables) {
        // Returning nullptr, no additonal Charsets available
        assert(variables.size() == 1);
        auto f = variables.variables[0];
        assert(f.type == Double);
        Variable result (Long);
        result.value.lv = reinterpret_cast<int64_t&> (f.value.fv);
        return result;
    });
    add("java/lang/Object", "getClass", "()Ljava/lang/Class;", [](const FunctionContext& context, const Variables& variables) {
        // Returning nullptr, no additonal Charsets available
        assert(variables.size() == 1);
        auto thisObject = variables.top();

        return context.interpreter->classByName(thisObject.value.object->type->name());
    });

    add("java/lang/Class", "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;", [](const FunctionContext& context, const Variables& variables) {
        assert(variables.size() == 3);
        std::string className = stringutils::dotToSlashNotatation(variables.variables[0].stringValue());


        bool initialize = variables.variables[1].value.iv != 0;
        return context.interpreter->classByName(className);
    });
    add("java/lang/Class", "newInstance", "()Ljava/lang/Object;", [](const FunctionContext& context, const Variables& variables) {
        assert(variables.size() == 1); // this pointer
        auto thisObject = variables.variables[0];
        assert(thisObject.type == ObjectRef);
        assert(thisObject.value.object->type->name() == "java/lang/Class");

        auto myClassNameVariable = thisObject.value.object->fields.find("__name");
        if (myClassNameVariable == thisObject.value.object->fields.end()){
            throw std::invalid_argument("Class object is not correctly initialized");
        }
        std::string className = myClassNameVariable->second.stringValue();

        ClassFilePtr classFile = context.loader->loadByName(className);
        Variable instance = context.memory->allocateObject(classFile);

        MethodIdentifier methodIdentifier;
        methodIdentifier.descriptor = "()V";
        methodIdentifier.methodName = "<init>";

        auto initMethod = classFile->methodWithSignature(methodIdentifier);
        assert((bool)(initMethod));

        Variables initArguments;
        initArguments.push(instance);
        context.interpreter->executeMethod(*classFile, *initMethod, *context.previousFrame, initArguments);
        return instance;
    });
    add("java/nio/Bits", "byteOrder", "()Ljava/nio/ByteOrder;", [](const FunctionContext& context, const Variables& variables) {
        assert (variables.size() == 0);
        auto byteOrderClass = context.interpreter->findInitializedClass("java/nio/ByteOrder");
        GlobalVariableIdentifier id;
        id.className = byteOrderClass->name();
        id.name = "LITTLE_ENDIAN";
        Variable littleEndian = context.memory->getGlobal(id);
        return littleEndian;
    });
    add("java/nio/Bits", "<clinit>", "()V", [](const FunctionContext& context, const Variables& variables) {
        assert (variables.size() == 0);
        // does calls to sun.misc.Unsafe which we do not support.
        return Variable();
    });

    add("java/lang/System", "loadLibrary", "(Ljava/lang/String;)V", [](const FunctionContext& context, const Variables& variables) {
        assert(variables.size() == 1);
        std::string libName = variables.variables[0].stringValue();
        logi("Skipping loading of library ", libName);
        return Variable();
    });
    add("java/lang/System", "setOut0", "(Ljava/io/PrintStream;)V", [](const FunctionContext& context, const Variables& variables) {
        assert(variables.size() == 1);
        Variable printStream = variables.variables[0];
        assert (printStream.type == ObjectRef);
        assert (printStream.value.object->type->name() == "java/io/PrintStream");
        GlobalVariableIdentifier id;
        id.className = "java/lang/System";
        id.name = "out";
        context.memory->putGlobal(id, printStream);
        return Variable();
    });
    add("java/io/FileOutputStream", "writeBytes", "([BIIZ)V", [](const FunctionContext& context, const Variables& variables) {
        assert(variables.size() == 5);
        Variable thisPointer = variables.variables[0];
        assert (thisPointer.type == ObjectRef);

        Variable byteArray = variables.variables[1];
        assert (byteArray.isArray());
        assert (byteArray.array()->type == Byte);

        Variable offset = variables.variables[2];
        assert (offset.isStoredAsInteger());

        Variable length = variables.variables[3];
        assert (length.isStoredAsInteger());

        Variable append = variables.variables[4];
        assert (append.isStoredAsInteger());

        Variable fd = *thisPointer.value.object->privateField("java/io/FileOutputStream", "fd");
        Variable realFd = *(fd.value.object->privateField("java/io/FileDescriptor", "fd"));
        assert (realFd.isStoredAsInteger());
        int cFd = realFd.value.iv;

        // ignore append
        for (int i = 0; i < length.value.iv; i++){
            int8_t byte = (int8_t) byteArray.array()->values[i + offset.value.iv].iv;
            write(cFd, &byte, 1);
        }

        return Variable();
    });
    add("java/lang/Class", "desiredAssertionStatus", "()Z", [](const FunctionContext& context, const Variables& variables) {
        assert(variables.size() == 1);
        Variable result (Integer);
        result.value.iv = 1;
        return result;
    });
}