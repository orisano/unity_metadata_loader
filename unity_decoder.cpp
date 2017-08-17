#include <fstream>
#include <ios>
#include <cstdlib>
#include <map>
#include <string>
#include <algorithm>
#include <il2cpp-metadata.h>
#include <vm/String.h>
#include <vm/MetadataCache.h>

template<typename T>
T* fromAddr(void *base, size_t offset) {
    return reinterpret_cast<T*>(static_cast<char*>(base) + offset);
}

int main() {
    using std::ifstream;
    using std::ofstream;
    using std::endl;
    using std::string;
    using std::map;
    using std::remove;
    using std::malloc;
    using std::free;
    
    ifstream metadata_file("./global-metadata.dat", std::ios::binary | std::ios::ate);

    size_t metadata_size = metadata_file.tellg();
    metadata_file.seekg(0, std::ios::beg);

    char* metadata = static_cast<char*>(malloc(metadata_size + 1u));
    metadata_file.read(metadata, metadata_size);
    metadata[metadata_size] = 0;

    auto header = reinterpret_cast<Il2CppGlobalMetadataHeader*>(metadata);
    assert(header->sanity == 0xFAB11BAF);

    auto usages_lists = fromAddr<Il2CppMetadataUsageList>(metadata, header->metadataUsageListsOffset);
    auto usage_pairs = fromAddr<Il2CppMetadataUsagePair>(metadata, header->metadataUsagePairsOffset);
    auto string_literals = fromAddr<Il2CppStringLiteral>(metadata, header->stringLiteralOffset);
    auto string_literal_data = fromAddr<char>(metadata, header->stringLiteralDataOffset);

    map<int, string> plain_strings;

    auto usage_pairs_size = header->metadataUsageListsCount / sizeof(Il2CppMetadataUsagePair);
    for (int i = 0; i < usage_pairs_size; i++) {
        auto usages = usages_lists[i];
        auto start = usages.start;
        auto size = usages.count;

        for (int j = 0; j < size; j++) {
            auto usage_pair = usage_pairs[start + j];
            auto destination_index = usage_pair.destinationIndex;
            auto encoded_source_index = usage_pair.encodedSourceIndex;
            auto usage = GetEncodedIndexType(encoded_source_index);
            auto decoded_index = GetDecodedMethodIndex(encoded_source_index);

            if (usage == kIl2CppMetadataUsageStringLiteral) {
                auto string_literal = string_literals[decoded_index];
                plain_strings[destination_index] = string(string_literal_data + string_literal.dataIndex, string_literal.length);
            }
        }
    }
    auto type_definitions = fromAddr<Il2CppTypeDefinition>(metadata, header->typeDefinitionsOffset);
    auto string_base = fromAddr<char>(metadata, header->stringOffset);
    auto methods = fromAddr<Il2CppMethodDefinition>(metadata, header->methodsOffset);

    map<int, string> method_names;

    auto type_size = header->typeDefinitionsCount / sizeof(Il2CppTypeDefinition);
    for (int i = 0; i < type_size; i++) {
        auto type_definition = type_definitions[i];
        auto type_name = string(string_base + type_definition.nameIndex);

        auto start = type_definition.methodStart;
        auto size = type_definition.method_count;
        for (int j = 0; j < size; j++) {
            auto method = methods[start + j];
            auto method_name = string(string_base + method.nameIndex);
            method_names[method.methodIndex] = type_name + "$$" + method_name;
        }
    }

    {
        ofstream ofs("./string_literal.txt");
        ofs << plain_strings.size() << endl;
        for (auto it = plain_strings.rbegin(); it != plain_strings.rend(); ++it) {
            auto sl = it->second;
            sl.erase(remove(sl.begin(), sl.end(), '\r'), sl.end());
            sl.erase(remove(sl.begin(), sl.end(), '\n'), sl.end());
            ofs << sl << endl;
        }
    }

    {
        ofstream ofs("./method_name.txt");
        ofs << method_names.size() << endl;
        for (auto it = method_names.rbegin(); it != method_names.rend(); ++it) {
            ofs << it->second << endl;
        }
    }
    free(metadata);
}
