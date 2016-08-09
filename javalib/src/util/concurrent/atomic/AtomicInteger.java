package java.util.concurrent.atomic;

/** Fake of Atomic Integer. */
class AtomicInteger {
    private int value = 0;

    public int getAndAdd(int v) {
        int previous = value;
        value += v;
        return previous;
    }
}