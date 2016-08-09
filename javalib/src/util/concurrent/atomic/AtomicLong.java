package java.util.concurrent.atomic;

/** Fake of Atomic Long. */
class AtomicLong {
    private long value = 0;

    public long getAndAdd(long v) {
        long previous = value;
        value += v;
        return previous;
    }
}