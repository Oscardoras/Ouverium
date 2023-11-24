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
        interface += "#include \"include.h\"\n";

        implementation += "#include <stddef.h>\n";
        implementation += "#include \"structures.h\"\n";
        implementation += "\n\n";

        for (auto const& structure : code.structures) {
            interface += "extern __VirtualTable __VirtualTable_" + structure->name + ";\n";

            implementation += "struct " + structure->name + " {\n";
            if (structure->function)
                implementation += "\t__Function __function;\n";
            if (structure->array.lock())
                implementation += "\t__Array __array;\n";
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
            if (auto array_type = structure->array.lock()) {
                implementation += "\t.array.vtable = &__VirtualTable_" + array_type->name + ",\n";
                implementation += "\t.array.offset = offsetof(struct " + structure->name + ", __array),\n";
            } else {
                implementation += "\t.array.vtable = NULL,\n";
                implementation += "\t.array.offset = NULL,\n";
            }
            if (structure->function)
                implementation += "\t.function.offset = offsetof(struct " + structure->name + ", __function),\n";
            else
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
        interface += "#include \"include.h\"\n";
        interface += "#include \"structures.h\"\n";

        implementation += "#include \"functions.h\"\n";
        implementation += "\n\n";

        for (auto const& function : code.functions) {
            if (function->filter.return_value != nullptr) {
                interface += "bool " + function->name + "_filter(__Reference_Owned captures[], __Reference_Shared args[]);\n";

                implementation += "bool " + function->name + "_filter(__Reference_Owned captures[], __Reference_Shared args[]) {\n";
                for (size_t i = 0; i < function->captures.size(); ++i) {
                    implementation += "\t__Reference_Owned " + function->captures[i].get_expression_code() + " = captures[" + std::to_string(i) + "];\n";
                }
                for (size_t i = 0; i < function->parameters.size(); ++i) {
                    implementation += "\t__Reference_Shared " + function->parameters[i].get_expression_code() + " = args[" + std::to_string(i) + "];\n";
                }
                for (auto const& [var, _] : function->filter.local_variables) {
                    implementation += "\t__UnknownData " + var + "_data = { .vtable = NULL, .data.ptr = NULL };\n";
                    implementation += "\t__Reference_Owned " + var + " = __Reference_new_symbol(" + var + "_data);\n";
                }
                for (auto const& instruction : function->filter.body) {
                    implementation += "\t" + instruction->get_instruction_code() + "\n";
                }
                implementation += "\treturn " + function->filter.return_value->get_expression_code() + ";\n";
                implementation += "}\n";
            }

            {
                interface += "__Reference_Owned " + function->name + "_body(__Reference_Owned captures[], __Reference_Shared args[]);\n";

                implementation += "__Reference_Owned " + function->name + "_body(__Reference_Owned captures[], __Reference_Shared args[]) {\n";
                for (size_t i = 0; i < function->captures.size(); ++i) {
                    implementation += "\t__Reference_Owned " + function->captures[i].get_expression_code() + " = captures[" + std::to_string(i) + "];\n";
                }
                for (size_t i = 0; i < function->parameters.size(); ++i) {
                    implementation += "\t__Reference_Shared " + function->parameters[i].get_expression_code() + " = args[" + std::to_string(i) + "];\n";
                }
                for (auto const& [var, _] : function->body.local_variables) {
                    implementation += "\t__UnknownData " + var + "_data = { .vtable = NULL, .data.ptr = NULL };\n";
                    implementation += "\t__Reference_Owned " + var + " = __Reference_new_symbol(" + var + "_data);\n";
                }
                for (auto const& instruction : function->body.body) {
                    implementation += "\t" + instruction->get_instruction_code() + "\n";
                }
                implementation += "\treturn " + function->body.return_value->get_expression_code() + ";\n";
                implementation += "}\n";
            }
        }
    }

    void Translator::write_main(std::string& implementation) {
        implementation += "int main(int, char**) {\n";
        implementation += "\t__GC_init();\n";
        for (auto const& instruction : code.main_instructions) {
            implementation += "\t" + instruction->get_instruction_code() + "\n";
        }
        implementation += "\t__GC_end();\n";
        implementation += "}\n";
    }

    std::string Reference::get_expression_code() const {
        return "reference" + std::to_string(number);
    }
    std::string Reference::get_instruction_code() const {
        return std::string() + "__Reference_" + (owned ? "Owned" : "Shared") + " reference" + std::to_string(number) + ";";
    }

    std::string Affectation::get_expression_code() const {
        std::string code;

        if (auto ref = std::dynamic_pointer_cast<Reference>(lvalue)) {
            if (ref->owned)
                code += "__Reference_Owned ";
            else
                code += "__Reference_Shared ";
        }
        code += lvalue->get_expression_code() + " = " + value->get_expression_code();

        return code;
    }
    std::string Affectation::get_instruction_code() const {
        return get_expression_code() + ";";
    }

    std::string Symbol::get_expression_code() const {
        std::string code;

        for (auto c : name) {
            switch (c) {
            case '!':
                code += "_x21";
                break;
            case '$':
                code += "_x24";
                break;
            case '%':
                code += "_x25";
                break;
            case '&':
                code += "_x26";
                break;
            case '*':
                code += "_x2A";
                break;
            case '+':
                code += "_x2B";
                break;
            case '-':
                code += "_x2D";
                break;
            case '/':
                code += "_x2F";
                break;
            case ':':
                code += "_x3A";
                break;
            case ';':
                code += "_x3B";
                break;
            case '<':
                code += "_x3C";
                break;
            case '=':
                code += "_x3D";
                break;
            case '>':
                code += "_x3E";
                break;
            case '?':
                code += "_x3F";
                break;
            case '@':
                code += "_x40";
                break;
            case '^':
                code += "_x5E";
                break;
            case '|':
                code += "_x7C";
                break;
            case '~':
                code += "_x7E";
                break;
            default:
                code += c;
                break;
            }
        }

        return code;
    }

    std::string Value::get_expression_code() const {
        return std::visit(Visitor{}, value);
    }

    std::string FunctionCall::get_expression_code() const {
        std::string code;

        if (std::dynamic_pointer_cast<Symbol>(function))
            code += function->get_expression_code() + "(";
        else
            code += "(" + function->get_expression_code() + ")(";
        size_t i = 0;
        for (auto const& p : parameters) {
            code += p->get_expression_code();
            ++i;
            if (i < parameters.size())
                code += ", ";
        }
        code += ")";

        return code;
    }
    std::string FunctionCall::get_instruction_code() const {
        return get_expression_code() + ";";
    }

    std::string Referencing::get_expression_code() const {
        return "&(" + expression->get_expression_code() + ")";
    }

    std::string Property::get_expression_code() const {
        return object->get_expression_code() + (pointer ? "->": ".") + name;
    }

    std::string List::get_expression_code() const {
        std::string code;

        code += "{ ";
        for (auto const& e : objects)
            code += e->get_expression_code() + ", ";
        code += "}";

        return code;
    }

    std::string FunctionExpression::get_expression_code() const {
        return "expression" + std::to_string(number);
    }
    std::string FunctionExpression::get_instruction_code() const {
        std::string code;

        if (auto tuple = std::get_if<std::vector<std::shared_ptr<FunctionExpression>>>(this)) {
            code += "__Expression expression" + std::to_string(number) + "_array = { ";
            for (auto const& e : *tuple)
                code += e->get_expression_code() + ", ";
            code += "};\n";
            code += "__Expression expression" + std::to_string(number) + " = { .type = __EXPRESSION_TUPLE, .tuple.size = " + std::to_string(tuple->size()) + ", .tuple.tab = expression" + std::to_string(number) + "_array };";
        } else if (auto ref = std::get_if<std::shared_ptr<Reference>>(this)) {
            code += "__Expression expression" + std::to_string(number) + " = { .type = __EXPRESSION_REFERENCE, .reference = __Reference_share(" + (*ref)->get_expression_code() + ") };";
        }

        return code;
    }

    std::string FunctionDefinition::get_expression_code() const {
        std::string code;

        code += "\"" + format + "\", " + name + "_body, " + name + "_filter, {";
        for (auto capture : captures)
            code += capture.get_expression_code() + ", ";
        code += "}, " + std::to_string(captures.size());

        return code;
    }

}
