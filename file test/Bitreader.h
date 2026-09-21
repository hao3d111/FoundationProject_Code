class BitReader {
private:
    ifstream *in;
    int buffer;
    int pos;
public:
    BitReader(ifstream *inStream) {
        in = inStream;
        buffer = 0;
        pos = 8;
    }
    int readBit() {
        if (pos == 8) {
            int c = in->get();
            if (c == -1) {
                return -1;
            }
            buffer = c;
            pos = 0;
        }
        int bit = (buffer >> (7 - pos)) & 1;
        pos = pos + 1;
        return bit;
    }
    string readBits(int n) {
        string result = "";
        for (int i = 0; i < n; i++) {
            int bit = readBit();
            if (bit == -1) {
                break;
            }
            if (bit == 1) {
                result = result + "1";
            } else {
                result = result + "0";
            }
        }
        return result;
    }
};
