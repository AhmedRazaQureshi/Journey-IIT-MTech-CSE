#include <iostream>
#include "qureshi.h"

using namespace std;

int main()
{
    string ext;
    cout << "Enter file type (.txt | .exe | .bmp | .jpg)" << endl;
    cout << "Extension of input file: ";
    cin >> ext;

    string inputFile = "input" + ext;
    string compressedFile = "compressed.z";
    string decompressedFile = "decompressed" + ext;

    if (ext == ".txt" || ext == ".exe" || ext == ".bmp" || ext == ".jpg")
    {
        // Compression
        compressFile(inputFile.c_str(), compressedFile.c_str());
        cout << "File compressed successfully." << endl;

        // Decompression
        decompressFile(compressedFile.c_str(), decompressedFile.c_str());
        cout << "File decompressed successfully." << endl;
    }
    else
    {
        cout << "Invalid file type" << endl;
        return 0;
    }

    return 0;
}
