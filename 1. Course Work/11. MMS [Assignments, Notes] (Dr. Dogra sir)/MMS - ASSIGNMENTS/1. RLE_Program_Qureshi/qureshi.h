// Support to RLE.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <divsufsort.h>
#include <zlib.h>

using namespace std;

// Node structure for Huffman tree
struct HuffmanNode
{
    char data;
    size_t frequency;
    HuffmanNode *left;
    HuffmanNode *right;

    HuffmanNode(char data, size_t frequency) : data(data), frequency(frequency), left(nullptr), right(nullptr) {}
};

// Comparison function for priority queue in Huffman Coding
struct CompareNodes
{
    bool operator()(HuffmanNode *left, HuffmanNode *right)
    {
        return left->frequency > right->frequency;
    }
};

// Function to read text from a file
string read_text_from_file(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: Unable to open file - " << filename << endl;
        exit(EXIT_FAILURE);
    }

    // Read the entire file content into a string
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    file.close();
    return content;
}

// Function to write text to a file
void write_text_to_file(const string &filename, const string &text)
{
    ofstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: Unable to open file - " << filename << endl;
        exit(EXIT_FAILURE);
    }

    file << text;
    file.close();
}

// Function to write binary data to a file
void write_binary_to_file(const string &filename, const vector<uint8_t> &data)
{
    ofstream file(filename, ios::binary);
    if (!file.is_open())
    {
        cerr << "Error: Unable to open file - " << filename << endl;
        exit(EXIT_FAILURE);
    }

    file.write(reinterpret_cast<const char *>(data.data()), data.size());
    file.close();
}

// Function to read binary data from a file
vector<uint8_t> read_binary_from_file(const string &filename)
{
    ifstream file(filename, ios::binary);
    if (!file.is_open())
    {
        cerr << "Error: Unable to open file - " << filename << endl;
        exit(EXIT_FAILURE);
    }

    // Determine the size of the file
    file.seekg(0, ios::end);
    size_t size = file.tellg();
    file.seekg(0, ios::beg);

    // Read the entire binary content into a vector
    vector<uint8_t> data(size);
    file.read(reinterpret_cast<char *>(data.data()), size);

    file.close();
    return data;
}

// Function to perform Burrows-Wheeler-Scott transform using DivSufSort library
string BWS_transform(const string &input)
{
    // Convert input string to byte array
    vector<uint8_t> byte_array(input.begin(), input.end());

    // Perform suffix array construction using DivSufSort
    vector<int> suffix_array(byte_array.size());
    divsufsort(byte_array.data(), suffix_array.data(), byte_array.size());

    // Construct the transformed string
    string transformed;
    for (size_t i = 0; i < suffix_array.size(); ++i)
    {
        int index = (suffix_array[i] > 0) ? suffix_array[i] - 1 : byte_array.size() - 1;
        transformed.push_back(byte_array[index]);
    }

    return transformed;
}

// Function to perform inverse Burrows-Wheeler-Scott transform
string inverse_BWS_transform(const string &transformed)
{
    // Size of the transformed string
    size_t size = transformed.size();

    // Create a vector of pairs to store the index and the corresponding character
    vector<pair<size_t, char>> indexed_chars(size);
    for (size_t i = 0; i < size; ++i)
    {
        indexed_chars[i] = make_pair(i, transformed[i]);
    }

    // Sort the vector of pairs based on the characters
    sort(indexed_chars.begin(), indexed_chars.end(), [](const auto &a, const auto &b)
         { return a.second < b.second; });

    // Find the index of the row that ends with the special character (last column in the sorted matrix)
    size_t special_char_index = 0;
    for (size_t i = 0; i < size; ++i)
    {
        if (indexed_chars[i].first == size - 1)
        {
            special_char_index = i;
            break;
        }
    }

    // Construct the original string using the sorted order
    string original;
    size_t current_index = special_char_index;
    for (size_t i = 0; i < size; ++i)
    {
        original.push_back(indexed_chars[current_index].second);
        current_index = indexed_chars[current_index].first;
    }

    return original;
}

// Function to perform Vertical Byte Reading
string vertical_byte_reading(const string &input)
{
    string result;

    for (size_t i = 0; i < input.length(); ++i)
    {
        for (size_t j = i; j < input.length(); j += input.length())
        {
            result.push_back(input[j]);
        }
    }

    return result;
}

// Function to perform Run-Length Encoding
string run_length_encode(const string &input)
{
    string encoded;

    for (size_t i = 0; i < input.length(); ++i)
    {
        char current_char = input[i];
        size_t count = 1;

        // Count consecutive occurrences of the current character
        while (i + 1 < input.length() && input[i + 1] == current_char)
        {
            ++count;
            ++i;
        }

        // Append the character and its count to the encoded string
        encoded.push_back(current_char);
        encoded.append(to_string(count));
    }

    return encoded;
}

// Function to perform Run-Length Decoding
string run_length_decode(const string &input)
{
    string decoded;

    for (size_t i = 0; i < input.length(); ++i)
    {
        char current_char = input[i];

        // Check if the next character is a digit (indicating a count)
        if (i + 1 < input.length() && isdigit(input[i + 1]))
        {
            // Extract the count from the string
            size_t count = stoi(input.substr(i + 1));

            // Append the current character 'count' times to the decoded string
            decoded.append(count, current_char);

            // Skip the digits in the input
            while (i + 1 < input.length() && isdigit(input[i + 1]))
            {
                ++i;
            }
        }
        else
        {
            // If no count is provided, append the current character once
            decoded.push_back(current_char);
        }
    }

    return decoded;
}

// Function to build the Huffman tree
HuffmanNode *build_huffman_tree(const unordered_map<char, size_t> &frequency_map)
{
    // Priority queue to store Huffman nodes
    priority_queue<HuffmanNode *, vector<HuffmanNode *>, CompareNodes> min_heap;

    // Create a leaf node for each character and push it to the priority queue
    for (const auto &pair : frequency_map)
    {
        min_heap.push(new HuffmanNode(pair.first, pair.second));
    }

    // Build the Huffman tree by repeatedly combining the two nodes with the lowest frequency
    while (min_heap.size() > 1)
    {
        HuffmanNode *left = min_heap.top();
        min_heap.pop();

        HuffmanNode *right = min_heap.top();
        min_heap.pop();

        HuffmanNode *combined_node = new HuffmanNode('$', left->frequency + right->frequency);
        combined_node->left = left;
        combined_node->right = right;

        min_heap.push(combined_node);
    }

    return min_heap.top();
}

// Function to generate Huffman codes
void generate_huffman_codes(HuffmanNode *root, const string &current_code, unordered_map<char, string> &huffman_codes)
{
    if (root->left == nullptr && root->right == nullptr)
    {
        huffman_codes[root->data] = current_code;
        return;
    }

    if (root->left != nullptr)
    {
        generate_huffman_codes(root->left, current_code + "0", huffman_codes);
    }

    if (root->right != nullptr)
    {
        generate_huffman_codes(root->right, current_code + "1", huffman_codes);
    }
}

// Function to encode a string using Huffman coding
string huffman_encode(const string &input, const unordered_map<char, string> &huffman_codes)
{
    string encoded;

    for (char c : input)
    {
        encoded += huffman_codes.at(c);
    }

    return encoded;
}

// Function to decode a string using Huffman coding
string huffman_decode(const string &encoded, const HuffmanNode *root)
{
    string decoded;
    const HuffmanNode *current = root;

    for (char bit : encoded)
    {
        if (bit == '0')
        {
            current = current->left;
        }
        else
        {
            current = current->right;
        }

        if (current->left == nullptr && current->right == nullptr)
        {
            decoded.push_back(current->data);
            current = root;
        }
    }

    return decoded;
}

// Function to compress data using zlib
vector<uint8_t> compress_data(const string &data)
{
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (deflateInit(&stream, Z_BEST_COMPRESSION) != Z_OK)
    {
        cerr << "Error: Unable to initialize zlib for compression." << endl;
        exit(EXIT_FAILURE);
    }

    stream.next_in = (Bytef *)data.data();
    stream.avail_in = data.size();

    const size_t buffer_size = 4096;
    vector<uint8_t> buffer(buffer_size);

    vector<uint8_t> compressed_data;

    do
    {
        stream.next_out = buffer.data();
        stream.avail_out = buffer_size;

        if (deflate(&stream, Z_FINISH) == Z_STREAM_ERROR)
        {
            cerr << "Error: zlib compression failed." << endl;
            deflateEnd(&stream);
            exit(EXIT_FAILURE);
        }

        size_t compressed_size = buffer_size - stream.avail_out;
        compressed_data.insert(compressed_data.end(), buffer.begin(), buffer.begin() + compressed_size);
    } while (stream.avail_out == 0);

    deflateEnd(&stream);

    return compressed_data;
}

// Function to decompress data using zlib
string decompress_data(const vector<uint8_t> &compressed_data)
{
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (inflateInit(&stream) != Z_OK)
    {
        cerr << "Error: Unable to initialize zlib for decompression." << endl;
        exit(EXIT_FAILURE);
    }

    stream.next_in = (Bytef *)compressed_data.data();
    stream.avail_in = compressed_data.size();

    const size_t buffer_size = 4096;
    vector<uint8_t> buffer(buffer_size);

    string decompressed_data;

    do
    {
        stream.next_out = buffer.data();
        stream.avail_out = buffer_size;

        if (inflate(&stream, Z_NO_FLUSH) == Z_STREAM_ERROR)
        {
            cerr << "Error: zlib decompression failed." << endl;
            inflateEnd(&stream);
            exit(EXIT_FAILURE);
        }

        size_t decompressed_size = buffer_size - stream.avail_out;
        decompressed_data.append(reinterpret_cast<const char *>(buffer.data()), decompressed_size);
    } while (stream.avail_out == 0);

    inflateEnd(&stream);

    return decompressed_data;
}

// Compression function
void compressFile(const char *inputFile, const char *compressedFile)
{
    ifstream ifs(inputFile, ios::binary);
    ofstream ofs(compressedFile, ios::binary);

    if (!ifs || !ofs)
    {
        cerr << "Error opening files." << endl;
        return;
    }

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK)
    {
        cerr << "Error initializing compression." << endl;
        return;
    }

    char inBuffer[1024];
    char outBuffer[1024];

    stream.next_in = reinterpret_cast<Bytef *>(inBuffer);
    stream.avail_in = 0;
    stream.next_out = reinterpret_cast<Bytef *>(outBuffer);
    stream.avail_out = sizeof(outBuffer);

    do
    {
        if (stream.avail_in == 0 && !ifs.eof())
        {
            ifs.read(inBuffer, sizeof(inBuffer));
            stream.avail_in = static_cast<uInt>(ifs.gcount());
            stream.next_in = reinterpret_cast<Bytef *>(inBuffer);
        }

        if (deflate(&stream, Z_NO_FLUSH) == Z_STREAM_ERROR)
        {
            cerr << "Error compressing data." << endl;
            deflateEnd(&stream);
            return;
        }

        size_t have = sizeof(outBuffer) - stream.avail_out;
        ofs.write(outBuffer, have);
        stream.next_out = reinterpret_cast<Bytef *>(outBuffer);
        stream.avail_out = sizeof(outBuffer);
    } while (stream.avail_in > 0 || !ifs.eof());

    int deflateResult;
    do
    {
        deflateResult = deflate(&stream, Z_FINISH);
        if (deflateResult == Z_STREAM_ERROR)
        {
            cerr << "Error finishing compression." << endl;
            deflateEnd(&stream);
            return;
        }

        size_t have = sizeof(outBuffer) - stream.avail_out;
        ofs.write(outBuffer, have);
        stream.next_out = reinterpret_cast<Bytef *>(outBuffer);
        stream.avail_out = sizeof(outBuffer);
    } while (deflateResult != Z_STREAM_END);

    deflateEnd(&stream);
}

// Decompression function
void decompressFile(const char *compressedFile, const char *decompressedFile)
{
    ifstream ifs(compressedFile, ios::binary);
    ofstream ofs(decompressedFile, ios::binary);

    if (!ifs || !ofs)
    {
        cerr << "Error opening files." << endl;
        return;
    }

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (inflateInit(&stream) != Z_OK)
    {
        cerr << "Error initializing decompression." << endl;
        return;
    }

    char inBuffer[1024];
    char outBuffer[1024];

    stream.next_in = reinterpret_cast<Bytef *>(inBuffer);
    stream.avail_in = 0;
    stream.next_out = reinterpret_cast<Bytef *>(outBuffer);
    stream.avail_out = sizeof(outBuffer);

    do
    {
        if (stream.avail_in == 0 && !ifs.eof())
        {
            ifs.read(inBuffer, sizeof(inBuffer));
            stream.avail_in = static_cast<uInt>(ifs.gcount());
            stream.next_in = reinterpret_cast<Bytef *>(inBuffer);
        }

        if (inflate(&stream, Z_NO_FLUSH) == Z_STREAM_ERROR)
        {
            cerr << "Error decompressing data." << endl;
            inflateEnd(&stream);
            return;
        }

        size_t have = sizeof(outBuffer) - stream.avail_out;
        ofs.write(outBuffer, have);
        stream.next_out = reinterpret_cast<Bytef *>(outBuffer);
        stream.avail_out = sizeof(outBuffer);
    } while (stream.avail_in > 0 || !ifs.eof());

    inflateEnd(&stream);
}