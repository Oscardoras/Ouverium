#include <hash_string.h>
#include <set>

#include "Translator.hpp"


namespace Translator::CStandard {

    auto get_size(std::set<std::string> const& properties) {
        size_t size = properties.size();

        std::set<uint32_t> hash_set;
        for (auto p : properties)
            hash_set.insert(hash(p.c_str()) % size);

        while (hash_set.size() < properties.size()) {
            ++size;
            hash_set = {};
            for (auto p : properties)
                hash_set.insert(hash(p.c_str()) % size);
        }

        std::vector<std::string> vector;
        for (auto p : properties)
            vector[hash(p.c_str()) % size] = p;
        return vector;
    }

    void Translator::write_structures(std::string& interface, std::string& implementation) {
        interface += "#include <include.h>";

        implementation += "#include <stddef.h>\n";
        implementation += "#include \"structures.hpp\"\n";
        implementation += "\n\n";

        for (auto const& structure : c.structures) {
            interface += "extern __VirtualTable __VirtualTable_" + structure->name + ";\n";

            implementation += "struct " + structure->name + " {\n";
            for (auto const& [p, _] : structure->properties)
                implementation += "\t__UnknownData " + p + ";\n";
            implementation += "};\n";

            implementation += "void __VirtualTable_" + structure->name + "_gc_iterator(void* ptr) {\n";
            implementation += "\tstruct " + structure->name + "* obj = (struct " + structure->name + "*) ptr;\n";
            implementation += "\n";
            for (auto const& [p, _] : structure->properties)
                implementation += "\t__GC_iterate(__VirtualTable_UnknownData.gc_iterator, obj->" + p + ")\n";
            implementation += "}\n";

            implementation += "__VirtualTable __VirtualTable_" + structure->name + " = {\n";
            implementation += "\t.size = sizeof(struct " + structure->name + "),\n";
            implementation += "\t.gc_iterator = __VirtualTable_" + structure->name + "_gc_iterator,\n";
            implementation += "\t.gc_destructor = NULL,\n";
            implementation += "\t.array.vtable = NULL,\n";
            implementation += "\t.array.offset = NULL,\n";
            implementation += "\t.function.offset = NULL,\n";
            std::set<std::string> properties;
            for (auto const& [p, _] : structure->properties)
                properties.insert(p);
            auto vector = get_size(properties);
            implementation += "\t.table.size = " + std::to_string(vector.size()) + ",\n";
            implementation += "\t.table.tab = {\n";
            for (auto p : vector)
                if (p != "")
                    implementation += "\t\t{ .hash = " + std::to_string(hash(p.c_str())) + ", .offset = offsetof(struct " + structure->name + ", " + p + ") },\n";
                else
                    implementation += "\t\t{ .hash = 0, .offset = 0 },\n";
            implementation += "\t}\n";
            implementation += "};\n";
        }
    }

    void Translator::write_functions(std::string& interface, std::string& implementation) {
        interface += "#include <include.h>";

        implementation += "#include \"functions.hpp\"\n";
        implementation += "\n\n";

        for (auto const& function : c.functions) {
            interface += "extern __VirtualTable __VirtualTable_" + structure->name + "();\n";

            implementation += "struct " + structure->name + " {\n";

            implementation += "}\n";
        }
    }

    void Translator::write_main(std::string& implementation) {

    }

}
