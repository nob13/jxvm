#pragma once
// Auto generated with build_ops.sh

namespace ops {

    // Constants
    const int nop = 0x00;
    const int aconst_null = 0x01;
    const int iconst_m1 = 0x02;
    const int iconst_0 = 0x03;
    const int iconst_1 = 0x04;
    const int iconst_2 = 0x05;
    const int iconst_3 = 0x06;
    const int iconst_4 = 0x07;
    const int iconst_5 = 0x08;
    const int lconst_0 = 0x09;
    const int lconst_1 = 0x0a;
    const int fconst_0 = 0x0b;
    const int fconst_1 = 0x0c;
    const int fconst_2 = 0x0d;
    const int dconst_0 = 0x0e;
    const int dconst_1 = 0x0f;
    const int bipush = 0x10;
    const int sipush = 0x11;
    const int ldc = 0x12;
    const int ldc_w = 0x13;
    const int ldc2_w = 0x14;

    // Loads
    const int iload = 0x15;
    const int lload = 0x16;
    const int fload = 0x17;
    const int dload = 0x18;
    const int aload = 0x19;
    const int iload_0 = 0x1a;
    const int iload_1 = 0x1b;
    const int iload_2 = 0x1c;
    const int iload_3 = 0x1d;
    const int lload_0 = 0x1e;
    const int lload_1 = 0x1f;
    const int lload_2 = 0x20;
    const int lload_3 = 0x21;
    const int fload_0 = 0x22;
    const int fload_1 = 0x23;
    const int fload_2 = 0x24;
    const int fload_3 = 0x25;
    const int dload_0 = 0x26;
    const int dload_1 = 0x27;
    const int dload_2 = 0x28;
    const int dload_3 = 0x29;
    const int aload_0 = 0x2a;
    const int aload_1 = 0x2b;
    const int aload_2 = 0x2c;
    const int aload_3 = 0x2d;
    const int iaload = 0x2e;
    const int laload = 0x2f;
    const int faload = 0x30;
    const int daload = 0x31;
    const int aaload = 0x32;
    const int baload = 0x33;
    const int caload = 0x34;
    const int saload = 0x35;

    // Stores
    const int istore = 0x36;
    const int lstore = 0x37;
    const int fstore = 0x38;
    const int dstore = 0x39;
    const int astore = 0x3a;
    const int istore_0 = 0x3b;
    const int istore_1 = 0x3c;
    const int istore_2 = 0x3d;
    const int istore_3 = 0x3e;
    const int lstore_0 = 0x3f;
    const int lstore_1 = 0x40;
    const int lstore_2 = 0x41;
    const int lstore_3 = 0x42;
    const int fstore_0 = 0x43;
    const int fstore_1 = 0x44;
    const int fstore_2 = 0x45;
    const int fstore_3 = 0x46;
    const int dstore_0 = 0x47;
    const int dstore_1 = 0x48;
    const int dstore_2 = 0x49;
    const int dstore_3 = 0x4a;
    const int astore_0 = 0x4b;
    const int astore_1 = 0x4c;
    const int astore_2 = 0x4d;
    const int astore_3 = 0x4e;
    const int iastore = 0x4f;
    const int lastore = 0x50;
    const int fastore = 0x51;
    const int dastore = 0x52;
    const int aastore = 0x53;
    const int bastore = 0x54;
    const int castore = 0x55;
    const int sastore = 0x56;

    // Stack
    const int pop = 0x57;
    const int pop2 = 0x58;
    const int dup = 0x59;
    const int dup_x1 = 0x5a;
    const int dup_x2 = 0x5b;
    const int dup2 = 0x5c;
    const int dup2_x1 = 0x5d;
    const int dup2_x2 = 0x5e;
    const int swap = 0x5f;

    // Math
    const int iadd = 0x60;
    const int ladd = 0x61;
    const int fadd = 0x62;
    const int dadd = 0x63;
    const int isub = 0x64;
    const int lsub = 0x65;
    const int fsub = 0x66;
    const int dsub = 0x67;
    const int imul = 0x68;
    const int lmul = 0x69;
    const int fmul = 0x6a;
    const int dmul = 0x6b;
    const int idiv = 0x6c;
    const int ldiv = 0x6d;
    const int fdiv = 0x6e;
    const int ddiv = 0x6f;
    const int irem = 0x70;
    const int lrem = 0x71;
    const int frem = 0x72;
    const int drem = 0x73;
    const int ineg = 0x74;
    const int lneg = 0x75;
    const int fneg = 0x76;
    const int dneg = 0x77;
    const int ishl = 0x78;
    const int lshl = 0x79;
    const int ishr = 0x7a;
    const int lshr = 0x7b;
    const int iushr = 0x7c;
    const int lushr = 0x7d;
    const int iand = 0x7e;
    const int land = 0x7f;
    const int ior = 0x80;
    const int lor = 0x81;
    const int ixor = 0x82;
    const int lxor = 0x83;
    const int iinc = 0x84;

    // Conversions
    const int i2l = 0x85;
    const int i2f = 0x86;
    const int i2d = 0x87;
    const int l2i = 0x88;
    const int l2f = 0x89;
    const int l2d = 0x8a;
    const int f2i = 0x8b;
    const int f2l = 0x8c;
    const int f2d = 0x8d;
    const int d2i = 0x8e;
    const int d2l = 0x8f;
    const int d2f = 0x90;
    const int i2b = 0x91;
    const int i2c = 0x92;
    const int i2s = 0x93;

    // Comparisons
    const int lcmp = 0x94;
    const int fcmpl = 0x95;
    const int fcmpg = 0x96;
    const int dcmpl = 0x97;
    const int dcmpg = 0x98;
    const int ifeq = 0x99;
    const int ifne = 0x9a;
    const int iflt = 0x9b;
    const int ifge = 0x9c;
    const int ifgt = 0x9d;
    const int ifle = 0x9e;
    const int if_icmpeq = 0x9f;
    const int if_icmpne = 0xa0;
    const int if_icmplt = 0xa1;
    const int if_icmpge = 0xa2;
    const int if_icmpgt = 0xa3;
    const int if_icmple = 0xa4;
    const int if_acmpeq = 0xa5;
    const int if_acmpne = 0xa6;

    // Control
    const int goto_ = 0xa7;
    const int jsr = 0xa8;
    const int ret = 0xa9;
    const int tableswitch = 0xaa;
    const int lookupswitch = 0xab;
    const int ireturn = 0xac;
    const int lreturn = 0xad;
    const int freturn = 0xae;
    const int dreturn = 0xaf;
    const int areturn = 0xb0;
    const int return_ = 0xb1;

    // References
    const int getstatic = 0xb2;
    const int putstatic = 0xb3;
    const int getfield = 0xb4;
    const int putfield = 0xb5;
    const int invokevirtual = 0xb6;
    const int invokespecial = 0xb7;
    const int invokestatic = 0xb8;
    const int invokeinterface = 0xb9;
    const int invokedynamic = 0xba;
    const int new_ = 0xbb;
    const int newarray = 0xbc;
    const int anewarray = 0xbd;
    const int arraylength = 0xbe;
    const int athrow = 0xbf;
    const int checkcast = 0xc0;
    const int instanceof = 0xc1;
    const int monitorenter = 0xc2;
    const int monitorexit = 0xc3;

    // Extended
    const int wide = 0xc4;
    const int multianewarray = 0xc5;
    const int ifnull = 0xc6;
    const int ifnonnull = 0xc7;
    const int goto_w = 0xc8;
    const int jsr_w = 0xc9;

    // Reserved
    const int breakpoint = 0xca;
    const int impdep1 = 0xfe;
    const int impdep2 = 0xff;
    // Convert op code into human string
    const char* opToStr(int op);
}
