#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <il2cpp-metadata.h>
#include <vm/String.h>
#include <vm/MetadataCache.h>

template<typename T>
T* fromAddr(void *base, size_t offset) {
    return reinterpret_cast<T*>(static_cast<char*>(base) + offset);
}

int main(int argc, char **argv) {
    using std::cerr;
    using std::cout;
    using std::hex;
    using std::endl;
    using std::atoi;
    using std::ifstream;
    using std::string;
    using std::malloc;
    using std::free;
    
    if (argc == 1) {
        cerr << "too few arguments" << endl;
        cerr << "Usage: inspector [type_index]" << endl;
        return 1;
    }
    int type_index = atoi(argv[1]);

    ifstream metadata_file("./global-metadata.dat", std::ios::binary | std::ios::ate);

    size_t metadata_size = metadata_file.tellg();
    metadata_file.seekg(0, std::ios::beg);

    char* metadata = static_cast<char*>(malloc(metadata_size + 1u));
    metadata_file.read(metadata, metadata_size);
    metadata[metadata_size] = 0;

    auto header = reinterpret_cast<Il2CppGlobalMetadataHeader*>(metadata);
    assert(header->sanity == 0xFAB11BAF);

    auto type_defs = fromAddr<Il2CppTypeDefinition>(metadata, header->typeDefinitionsOffset);
    auto type_def = type_defs[type_index];

    auto fields = fromAddr<Il2CppFieldDefinition>(metadata, header->fieldsOffset);

    auto names = fromAddr<char>(metadata, header->stringOffset);

    auto field_default_values = fromAddr<Il2CppFieldDefaultValue>(metadata, header->fieldDefaultValuesOffset);
    auto field_default_values_size = header->fieldDefaultValuesCount / sizeof(Il2CppFieldDefaultValue);

    cout << string(names + type_def.nameIndex) << ":" << endl;
    for (int i = 0; i < type_def.field_count; i++) {
        int idx = type_def.fieldStart + i;
        auto field = fields[idx];
        cout << "\t" << string(names + field.nameIndex) << " = ";
        for (int j = 0; j < field_default_values_size; j++) {
            auto fdv = field_default_values[j];
            if (fdv.fieldIndex == idx) {
                int addr = header->fieldAndParameterDefaultValueDataOffset + fdv.dataIndex;
                cout << *fromAddr<int>(metadata, addr);
                cout << "; // offset: " << hex << addr;
                goto FOUND;
            }
        }
        cout << "?";
FOUND:;
      cout << endl;
    }

    free(metadata);
}
