#include <iostream>
#include <string>
#include <fstream>
using namespace std;

const string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

string base64_encode(const string& input) {
    string encoded;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (encoded.size() % 4) encoded.push_back('=');
    return encoded;
}

string base64_decode(const string& input) {
    string decoded;
    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (c == '=') break;
        int pos = base64_chars.find(c);
        if (pos == string::npos) continue;
        val = (val << 6) + pos;
        valb += 6;
        if (valb >= 0) {
            decoded.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return decoded;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage: " << argv[0] << " <encode|decode> <input_file> <output_file>" << endl;
        return 1;
    }
    
    string mode = argv[1];
    string input_file = argv[2];
    string output_file = argv[3];
    
    ifstream file(input_file);
    if (!file) {
        cout << "Error: Cannot open file " << input_file << endl;
        return 1;
    }
    
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    
    ofstream outfile(output_file);
    if (!outfile) {
        cout << "Error: Cannot create output file " << output_file << endl;
        return 1;
    }
    
    if (mode == "encode") {
        outfile << base64_encode(content);
    } else if (mode == "decode") {
        outfile << base64_decode(content);
    } else {
        cout << "Error: Mode must be 'encode' or 'decode'" << endl;
        return 1;
    }
    
    return 0;
}
