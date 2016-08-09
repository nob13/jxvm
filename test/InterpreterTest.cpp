#include <gtest/gtest.h>

#include <jx/Interpreter.h>

struct InterpreterTest : public testing::Test {
    InterpreterTest(){
        interpreter.classLoader().addDefaultPaths();
        interpreter.classLoader().addPath(util::executableDirectory() + "/../lib/test.jar");
    }

    Interpreter interpreter;
};

TEST_F(InterpreterTest, assertion){
    Variables variables;
    bool catched = false;
    try {
        interpreter.callStatic("jx/test/InterpreterTest", "assertion", variables);
    } catch (JvmException & e){
        catched = true;
        //ok
    }
    ASSERT_TRUE(catched);
}

TEST_F (InterpreterTest, leftShiftTest){
    Variables variables;
    Variable retValue = interpreter.callStatic("jx/test/InterpreterTest", "leftShiftTest", variables);
    ASSERT_EQ(Integer, retValue.type);
    ASSERT_EQ(-671088640, retValue.value.iv);
}

TEST_F (InterpreterTest, emptyHashCodeTest){
    Variables variables;
    Variable retValue = interpreter.callStatic("jx/test/InterpreterTest", "emptyHashCodeTest", variables);
    ASSERT_EQ(Integer, retValue.type);
    ASSERT_EQ(0, retValue.value.iv);
}


TEST_F (InterpreterTest, shortHashCodeTest){
    Variables variables;
    Variable retValue = interpreter.callStatic("jx/test/InterpreterTest", "shortHashCodeTest", variables);
    ASSERT_EQ(Integer, retValue.type);
    ASSERT_EQ(3105, retValue.value.iv);
}

TEST_F (InterpreterTest, hashCodeTest){
    Variables variables;
    Variable retValue = interpreter.callStatic("jx/test/InterpreterTest", "helloHashCode", variables);
    ASSERT_EQ(Integer, retValue.type);
    ASSERT_EQ(439329280, retValue.value.iv);
}

TEST_F (InterpreterTest, concatenatedStringTest){
    Variables variables;
    Variable retValue = interpreter.callStatic("jx/test/InterpreterTest", "concatenatedStringTest", variables);
    ASSERT_EQ("Hello World", retValue.stringValue());
}

TEST_F (InterpreterTest, handConcatenatedStringTest){
    Variables variables;
    Variable retValue = interpreter.callStatic("jx/test/InterpreterTest", "handConcatenatedStringTest", variables);
    ASSERT_EQ("Hello World", retValue.stringValue());
}

TEST_F (InterpreterTest, simpleMathTest){
    Variables variables;
    Variable retValue = interpreter.callStatic("jx/test/InterpreterTest", "simpleMathTest", variables);
    ASSERT_EQ(None, retValue.type);
}


TEST_F (InterpreterTest, simpleRemainderTest){
    Variables variables;
    Variable retValue = interpreter.callStatic("jx/test/InterpreterTest", "simpleRemainderTest", variables);
    ASSERT_EQ(None, retValue.type);
}

TEST_F (InterpreterTest, simpleShiftTest){
    Variables variables;
    Variable retValue = interpreter.callStatic("jx/test/InterpreterTest", "simpleShiftTest", variables);
    ASSERT_EQ(None, retValue.type);
}
