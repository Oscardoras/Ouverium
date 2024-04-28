#include <set>

#include <ouverium/hash_string.h>

#include "Translator.hpp"

#include "../../Types.hpp"


namespace Translator::CStandard {

    std::string Name::get() const {
        std::string code;

        for (auto c : symbol) {
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

        if (std::set<std::string>{"if", "else", "while"}.contains(code))
            code += "_statement";

        return code;
    }

    auto get_size(std::set<Name> const& properties) {
        size_t size = properties.size();

        std::set<uint32_t> hash_set;
        for (auto p : properties)
            hash_set.insert(hash_string(p.symbol.c_str()) % size);

        while (hash_set.size() < properties.size()) {
            ++size;
            hash_set = {};
            for (auto p : properties)
                hash_set.insert(hash_string(p.symbol.c_str()) % size);
        }

        std::vector<Name> vector(size);
        for (auto p : properties)
            vector[hash_string(p.symbol.c_str()) % size] = p;
        return vector;
    }

    void Translator::write_structures(std::string& interface, std::string& implementation) {
        interface += "#include <include.h>\n";

        implementation += "#include <stddef.h>\n";
        implementation += "#include \"structures.h\"\n";
        implementation += "\n\n";

        for (auto const& structure : code.structures) {
            interface += "extern Ov_VirtualTable Ov_VirtualTable_" + structure->name.get() + ";\n";

            implementation += "struct " + structure->name.get() + " {\n";
            if (structure->function)
                implementation += "\tOv_Function Ov_function;\n";
            if (structure->array.lock())
                implementation += "\tOv_Array Ov_array;\n";
            for (auto const& [p, _] : structure->properties)
                implementation += "\tOv_UnknownData " + p.get() + ";\n";
            implementation += "};\n";

            implementation += "void Ov_VirtualTable_" + structure->name.get() + "_gc_iterator(void* ptr) {\n";
            implementation += "\tstruct " + structure->name.get() + "* obj = (struct " + structure->name.get() + "*) ptr;\n";
            implementation += "\n";
            for (auto const& [p, _] : structure->properties)
                implementation += "\tOv_GC_iterate(Ov_VirtualTable_UnknownData.gc_iterator, &obj->" + p.get() + ");\n";
            implementation += "}\n";

            implementation += "Ov_VirtualTable Ov_VirtualTable_" + structure->name.get() + " = {\n";
            implementation += "\t.size = sizeof(struct " + structure->name.get() + "),\n";
            implementation += "\t.gc_iterator = Ov_VirtualTable_" + structure->name.get() + "_gc_iterator,\n";
            if (auto array_type = structure->array.lock()) {
                implementation += "\t.array.vtable = &Ov_VirtualTable_" + array_type->name.get() + ",\n";
                implementation += "\t.array.offset = offsetof(struct " + structure->name.get() + ", Ov_array),\n";
            } else {
                implementation += "\t.array.vtable = NULL,\n";
                implementation += "\t.array.offset = -1,\n";
            }
            if (structure->function)
                implementation += "\t.function.offset = offsetof(struct " + structure->name.get() + ", Ov_function),\n";
            else
                implementation += "\t.function.offset = -1,\n";
            std::set<Name> properties;
            for (auto const& [p, _] : structure->properties)
                properties.insert(p);
            auto vector = get_size(properties);
            implementation += "\t.table_size = " + std::to_string(vector.size()) + ",\n";
            implementation += "\t.table_tab = {\n";
            for (auto p : vector)
                if (p.symbol != "")
                    implementation += "\t\t{ .hash = " + std::to_string(hash_string(p.symbol.c_str())) + ", .offset = offsetof(struct " + structure->name.get() + ", " + p.get() + ") },\n";
                else
                    implementation += "\t\t{ .hash = 0, .offset = 0 },\n";
            implementation += "\t}\n";
            implementation += "};\n";
        }

        implementation += "\n";
        implementation += "Ov_VirtualTable* Ov_VirtualTable_string_from_tuple = &Ov_VirtualTable_Object;";
    }

    void Translator::write_functions(std::string& interface, std::string& implementation) {
        interface += "#include <include.h>\n";
        interface += "#include \"structures.h\"\n";

        implementation += "#include \"functions.h\"\n";
        implementation += "\n\n";

        for (auto const& lambda : code.lambdas) {
            implementation += "Ov_Reference_Owned lambda_" + std::to_string(lambda->get_id()) + "_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {\n";
            for (size_t i = 0; i < lambda->captures.size(); ++i)
                implementation += "\tOv_Reference_Shared " + lambda->captures[i].first.get() + " = captures[" + std::to_string(i) + "];\n";
            for (auto const& instruction : lambda->body.body)
                implementation += "\t" + instruction->get_instruction_code() + "\n";
            implementation += "\treturn " + lambda->body.return_value->get_expression_code() + ";\n";
            implementation += "}\n";
        }

        for (auto const& function : code.functions) {
            if (function->filter.return_value != nullptr) {
                interface += "bool " + function->name.get() + "_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]);\n";

                implementation += "bool " + function->name.get() + "_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {\n";
                for (size_t i = 0; i < function->captures.size(); ++i) {
                    implementation += "\tOv_Reference_Shared " + function->captures[i].first.get() + " = captures[" + std::to_string(i) + "];\n";
                }
                for (size_t i = 0; i < function->parameters.size(); ++i) {
                    implementation += "\tOv_Reference_Shared " + function->parameters[i].first.get() + " = args[" + std::to_string(i) + "];\n";
                }
                size_t i = 0;
                for (auto const& [var, _] : function->local_variables) {
                    implementation += "\tOv_Reference_Shared " + var.get() + " = local_variables[" + std::to_string(i) + "];\n";
                    ++i;
                }

                for (auto const& instruction : function->filter.body)
                    implementation += "\t" + instruction->get_instruction_code() + "\n";
                implementation += "\treturn " + function->filter.return_value->get_expression_code() + ";\n";

                implementation += "}\n";
            }

            {
                interface += "Ov_Reference_Owned " + function->name.get() + "_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]);\n";

                implementation += "Ov_Reference_Owned " + function->name.get() + "_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {\n";
                for (size_t i = 0; i < function->captures.size(); ++i) {
                    implementation += "\tOv_Reference_Shared " + function->captures[i].first.get() + " = captures[" + std::to_string(i) + "];\n";
                }
                for (size_t i = 0; i < function->parameters.size(); ++i) {
                    implementation += "\tOv_Reference_Shared " + function->parameters[i].first.get() + " = args[" + std::to_string(i) + "];\n";
                }
                size_t i = 0;
                for (auto const& [var, _] : function->local_variables) {
                    implementation += "\tOv_Reference_Shared " + var.get() + " = local_variables[" + std::to_string(i) + "];\n";
                    ++i;
                }

                for (auto const& instruction : function->body.body)
                    implementation += "\t" + instruction->get_instruction_code() + "\n";
                implementation += "\treturn " + function->body.return_value->get_expression_code() + ";\n";

                implementation += "}\n";
            }
        }
    }

    void Translator::write_main(std::string& implementation) {
        implementation += "int main(int, char**) {\n";
        implementation += "\tOv_init();\n";

        for (auto const& [var, _] : code.main.global_variables) {
            if (!symbols.contains(var.symbol)) {
                implementation += "\tOv_UnknownData " + var.get() + "_data = { .vtable = &Ov_VirtualTable_Object, .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object) };\n";
                implementation += "\tOv_Reference_Owned " + var.get() + " = Ov_Reference_new_symbol(" + var.get() + "_data);\n";
            }
        }

        for (auto const& instruction : code.main.body)
            implementation += "\t" + instruction->get_instruction_code() + "\n";

        // implementation += "\tint return_value = Ov_Reference_get(Ov_Reference_share(" + code.main.return_value->get_expression_code() + ")).data.i;\n";
        implementation += "\tOv_end();\n";
        // implementation += "\treturn return_value;\n";
        implementation += "\treturn 0;\n";
        implementation += "}\n";
    }

    std::string Declaration::get_instruction_code() const {
        return type + " " + symbol->get_expression_code() + ";";
    }

    std::string Reference::get_expression_code() const {
        return "reference" + std::to_string(id);
    }
    std::string Reference::get_instruction_code() const {
        return std::string() + "Ov_Reference_" + (owned ? "Owned" : "Shared") + " reference" + std::to_string(id) + ";";
    }

    std::string Affectation::get_expression_code() const {
        std::string code;

        if (auto ref = std::dynamic_pointer_cast<Reference>(lvalue)) {
            if (ref->owned)
                code += "Ov_Reference_Owned ";
            else
                code += "Ov_Reference_Shared ";
        }
        code += lvalue->get_expression_code() + " = " + value->get_expression_code();

        return code;
    }
    std::string Affectation::get_instruction_code() const {
        return get_expression_code() + ";";
    }

    std::string Symbol::get_expression_code() const {
        return name.get();
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
        return object->get_expression_code() + (pointer ? "->" : ".") + name.get();
    }

    std::string List::get_expression_code() const {
        if (objects.empty())
            return "NULL";
        else
            return "array" + std::to_string(id);
    }

    std::string List::get_instruction_code() const {
        std::string code;

        code += "Ov_Reference_Shared array" + std::to_string(id) + "[] = { ";
        for (auto const& e : objects)
            code += "Ov_Reference_share(" + e->get_expression_code() + "), ";
        code += "};";

        return code;
    }

    std::string FunctionExpression::get_expression_code() const {
        return "expression" + std::to_string(id);
    }
    std::string FunctionExpression::get_instruction_code() const {
        std::string code;

        if (auto tuple = std::get_if<std::vector<std::shared_ptr<FunctionExpression>>>(this)) {
            for (auto const& e : *tuple)
                code += e->get_instruction_code() + "\n\t";

            code += "Ov_Expression expression" + std::to_string(id) + "_array[] = { ";
            for (auto const& e : *tuple)
                code += e->get_expression_code() + ", ";
            code += "};\n";
            code += "\tOv_Expression expression" + std::to_string(id) + " = { .type = Ov_EXPRESSION_TUPLE, .tuple.size = " + std::to_string(tuple->size()) + ", .tuple.tab = expression" + std::to_string(id) + "_array };";
        } else if (auto ref = std::get_if<std::shared_ptr<Reference>>(this)) {
            code += "Ov_Expression expression" + std::to_string(id) + " = { .type = Ov_EXPRESSION_REFERENCE, .reference = Ov_Reference_share(" + (*ref)->get_expression_code() + ") };";
        } else if (auto lambda = std::get_if<std::shared_ptr<Lambda>>(this)) {
            code += "Ov_Expression expression" + std::to_string(id) + " = { .type = Ov_EXPRESSION_LAMBDA, .lambda = NULL };\n";

            if (!(*lambda)->captures.empty()) {
                code += "\tOv_Reference_Shared lambda_" + std::to_string((*lambda)->get_id()) + "_captures[] = { ";
                for (auto const& capture : (*lambda)->captures)
                    code += "Ov_Reference_share(" + capture.first.get() + "), ";
                code += "};\n";
            }

            code += "\tOv_Function_push(&expression" + std::to_string(id) + ".lambda, \"\", lambda_" + std::to_string((*lambda)->get_id()) + "_body, NULL, 0, " + ((*lambda)->captures.empty() ? "NULL" : ("lambda_" + std::to_string((*lambda)->get_id()) + "_captures")) + ", " + std::to_string((*lambda)->captures.size()) + ");";
        }

        return code;
    }

}
