#include <iostream>
#include <fstream>
#include "BitWriter.h"
#include "BitReader.h"
using namespace std;
int main() {
string original = "101100111010001110110001111";
ofstream outFile("test.bin", ios::binary);
BitWriter writer(outFile);
writer.writeBits(original);
writer.flush();
long totalBits = writer.getTotalBits();
outFile.close();
cout << "Đã ghi " << totalBits << " bit vao trong file" << endl;
ifstream inFile("test.bin", ios::binary);
BitReader reader(inFile);
string result = reader.readBits((int)original.size());
inFile.close();
cout << "GỐC : " << original << endl;
cout << "ĐỌC : " << result << endl;
if (original == result) {
cout << "Giống nhau -> code chạy đúng" << endl;
} else {
cout << "Khác nhau -> xảy ra lỗi" << endl;
}
return 0;
}
