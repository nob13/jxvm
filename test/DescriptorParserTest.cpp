#include <gtest/gtest.h>
#include <jx/DescriptorParser.h>
#include <jx/Interpreter.h>

TEST(DescriptorParserTest, isMethod) {
    DescriptorParser p("(Ljava/util/Properties;)V");
    ASSERT_TRUE(p.isMethod());
    DescriptorParser p2("(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    ASSERT_TRUE(p2.isMethod());
    DescriptorParser p3("(I)V");
    ASSERT_TRUE(p3.isMethod());
    DescriptorParser p4("()V");
    ASSERT_TRUE(p4.isMethod());
    DescriptorParser p5("()[C");
    ASSERT_TRUE(p5.isMethod());
}

TEST(DescriptorParserTest, isNoMethod) {
    DescriptorParser p("(Hallo");
    ASSERT_FALSE(p.isMethod());
}

TEST(DescriptorParserTest, getType) {
    DescriptorParser p("Ljava/io/InputStream");
    ASSERT_EQ(VariableType::ObjectRef, p.type());
    DescriptorParser p1("(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    ASSERT_EQ(VariableType::ObjectRef, p1.type());
    DescriptorParser p2("(Ljava/lang/String;Ljava/lang/String;)Z;");
    ASSERT_EQ(VariableType::Boolean, p2.type());
    DescriptorParser p3("[C");
    ASSERT_EQ(VariableType::ArrayRef, p3.type());
}

TEST(DescriptorParserTest, argumentCount) {
    DescriptorParser p("(IDLjava/lang/Thread;)Ljava/lang/Object");
    ASSERT_EQ(3, p.argumentCount());
    DescriptorParser p1("(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    ASSERT_EQ(2, p1.argumentCount());
    DescriptorParser p2("(Ljava/lang/String;Ljava/lang/String;ZF)Ljava/lang/String;");
    ASSERT_EQ(4, p2.argumentCount());
    DescriptorParser p3("([CJ)V");
    ASSERT_EQ(2, p3.argumentCount());

    DescriptorParser p4("(Ljava/lang/Class;[Ljava/lang/String;)V");
    ASSERT_EQ(2, p4.argumentCount());
}

TEST(DescriptorParserTest, argumentType) {
    DescriptorParser p("(IDLjava/lang/Thread;)Ljava/lang/Object");
    ASSERT_EQ(VariableType::Integer, p.argument(0));
    ASSERT_EQ(VariableType::Double, p.argument(1));
    ASSERT_EQ(VariableType::ObjectRef, p.argument(2));
    DescriptorParser p1("(Ljava/lang/String;Ljava/lang/String;ZF)Ljava/lang/String;");
    ASSERT_EQ(VariableType::ObjectRef, p1.argument(0));
    ASSERT_EQ(VariableType::ObjectRef, p1.argument(1));
    ASSERT_EQ(VariableType::Boolean, p1.argument(2));
    ASSERT_EQ(VariableType::Float, p1.argument(3));
    DescriptorParser p3("([CJ)V");
    ASSERT_EQ(2, p3.argumentCount());
    ASSERT_EQ(VariableType::ArrayRef, p3.argument(0));
    ASSERT_EQ(VariableType::Long, p3.argument(1));
    ASSERT_EQ(VariableType::None, p3.type());
}

TEST(DescriptorParserTest, Whatever) {
    DescriptorParser p("(IDLjava/lang/Thread;)Ljava/lang/Object");
    ASSERT_TRUE(p.isMethod());
    ASSERT_EQ(3, p.argumentCount());
    ASSERT_EQ(VariableType::Integer, p.argument(0));
    ASSERT_EQ(VariableType::ObjectRef, p.type());
}


