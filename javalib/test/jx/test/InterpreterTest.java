package jx.test;

class InterpreterTest {

    public static void assertion(){
        assert false : "This must fail";
    }

    public static int leftShiftTest() {
        return 3254 << 58;
    }

    public static int emptyHashCodeTest() {
        return "".hashCode();
    }

    public static int shortHashCodeTest() {
        return "ab".hashCode();
    }

    public static int helloHashCode() {
        return "HelloWorld".hashCode();
    }

    public static String concatenatedStringTest() {
        return "Hello " + "World";
    }

    public static String handConcatenatedStringTest () {
        StringBuilder builder = new StringBuilder();
        builder.append("Hello");
        builder.append(' ');
        builder.append("World");
        return builder.toString();
    }

    public static void simpleMathTest(){
        assert 2 == 1 + 1;
        assert 2.0 == 1.0 + 1.0;

        assert 12.0 == 3.0 * 4.0;

        // Conversions
        assert 1.0f == (float) 1.0;
        assert 1.0 == toDouble(1);
        assert 1.0 == toDouble(1L);
        assert 1.0 == toDouble(1.0f);
        assert 1L == toLong(1.0f);
        assert 1L == toLong(1.0);
        assert 1L == toLong(1);
        assert 1 == toInt(1.0f);
        assert 1 == toInt(1.0);

        assert 1.0 == toDouble(1);

        // Floating point operations
        {
            float a = toFloat(3) + toFloat(4);
            assert a == 7.0f;
        }
        {
            float a = toFloat(4) / toFloat(2);
            assert a == 2.0f;
        }
        {
            float a = toFloat(4) - toFloat(7);
            assert a == -3.0f;
        }
        {
            float a = toFloat(4) * toFloat(-2);
            assert a == -8.0f;
        }
        {
            float a = -(toFloat(4));
            assert a == -4.0f;
        }
        // Double operations
        {
            double a = toDouble(3) + toDouble(4);
            assert a == 7.0;
        }
        {
            double a = toDouble(4) / toDouble(2);
            assert a == 2.0;
        }
        {
            double a = toDouble(4) - toDouble(7);
            assert a == -3.0;
        }
        {
            double a = toDouble(4) * toDouble(-2);
            assert a == -8.0;
        }
        {
            double a = -(toDouble(4));
            assert a == -4.0;
        }
        // Integer operations
        {
            int a = toInt(3L) + toInt(4L);
            assert a == 7;
        }
        {
            int a = toInt(4L) / toInt(2L);
            assert a == 2;
        }
        {
            int a = toInt(4L) - toInt(7L);
            assert a == -3;
        }
        {
            int a = toInt(4L) * toInt(-2L);
            assert a == -8;
        }
        {
            int a = -(toInt(4L));
            assert a == -4;
        }
        // Long operations
        {
            long a = toLong(3) + toLong(4);
            assert a == 7L;
        }
        {
            long a = toLong(4) / toLong(2);
            assert a == 2L;
        }
        {
            long a = toLong(4) - toLong(7);
            assert a == -3L;
        }
        {
            long a = toLong(4) * toLong(-2);
            assert a == -8L;
        }
        {
            long a = -(toLong(4));
            assert a == -4L;
        }
    }

    private static void simpleRemainderTest(){
        {
            int rem = toInt(8L) % toInt(5L);
            assert (rem == 3);
        }
        {
            long rem = toLong(8) % toInt(5);
            assert (rem == 3L);
        }
        {
            float rem = toFloat(9.0) % toFloat(3.5);
            assert (rem == 2.0f);
        }
        {
            double rem = toDouble(9) % toDouble(3.5f);
            assert (rem == 2.0);
        }
    }

    private static void simpleShiftTest(){
        {
            long v = toLong(3) << toLong(35);
            assert v == 103079215104L;
        }
        {
            int v = toInt(3) << toInt(35);
            assert v == 24;
        }
        {
            long v = toLong(103079215104.0) >> toLong(35);
            assert v == 3;
        }
        {
            int v = toInt(48L) >> toInt(3L);
            assert v == 6;
        }
        {
            int v = toInt(-48L) >>> toInt(3L);
            assert (v == 536870906);
        }
        {
            int v = toInt(-48L) >> toInt(3L);
            assert (v == -6);
        }
    }

    private static float toFloat(int v){
        return (float)v;
    }

    private static float toFloat(double v){
        return (float) v;
    }

    private static double toDouble(int v){
        return (double)v;
    }

    private static double toDouble(long v){
        return (double)v;
    }

    private static double toDouble(float v){
        return (double)v;
    }


    private static int toInt(double v){
        return (int) v;
    }

    private static int toInt(float v){
        return (int) v;
    }

    private static long toLong(int v){
        return (long) v;
    }

    private static long toLong(float v){
        return (long) v;
    }

    private static long toLong(double v){
        return (long) v;
    }
}