class BitWriter {
private:
    ofstream *out;
    int buffer;
    int count;
    long totalBits;
public:
    BitWriter(ofstream *outStream) {
        out = outStream;
        buffer = 0;
        count = 0;
        totalBits = 0;
    }
    void writeBit(int bit) {
        buffer = buffer << 1;
        buffer = buffer | bit;
        count = count + 1;
        totalBits = totalBits + 1;
        if (count == 8) {
            out->put((char)buffer);
            buffer = 0;
            count = 0;
        }
    }
    void writeBits(string bits) {
        for (int i = 0; i < (int)bits.length(); i++) {
            if (bits[i] == '1') {
                writeBit(1);
            } else {
                writeBit(0);
            }
        }
    }
    long getTotalBits() {
        return totalBits;
    }
    void flush() {
        if (count > 0) {
            buffer = buffer << (8 - count);
            out->put((char)buffer);
            buffer = 0;
            count = 0;
        }
        out->flush();
    }
};
