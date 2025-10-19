#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include <ouverium/hash_string.h>

#include <ouverium/compiler/c/Code.hpp>
#include <ouverium/compiler/c/Translator.hpp>

#include <ouverium/types.h>


namespace Translator::CStandard {

    std::string add_indentation(std::string const& instructions) {
        std::string code;

        std::istringstream iss(instructions);
        std::string line;
        while (std::getline(iss, line))
            code += "\t" + line + "\n";

        return code;
    }

    std::string Name::get() const {
        std::string code;

        for (auto c : symbol) {
            switch (c) {
                case '.':
                    code += "_x2E";
                    break;
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

        if (std::set<std::string>{"if", "else", "while", "for", "default", "static"}.contains(code))
            code += "_keyword";

        return code;
    }

    auto get_size(std::set<Name> const& properties) {
        size_t size = properties.size();

        std::set<uint32_t> hash_set;
        for (auto const& p : properties)
            hash_set.insert(hash_string(p.symbol.c_str()) % size);

        while (hash_set.size() < properties.size()) {
            ++size;
            hash_set = {};
            for (auto const& p : properties)
                hash_set.insert(hash_string(p.symbol.c_str()) % size);
        }

        std::vector<Name> vector(size);
        for (auto const& p : properties)
            vector[hash_string(p.symbol.c_str()) % size] = p;
        return vector;
    }

    void Translator::write_structures(std::string& interface, std::string& implementation) {
        interface += "#include <include.h>\n";
        interface += "\n\n";

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
            for (auto const& p : vector)
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

    void Translator::write_global_variables(std::string& interface, std::string& implementation) {
        for (auto const& [name, type] : code.global_variables) {
            interface += "extern Ov_Reference_Owned " + name.get() + ";\n";
            implementation += "Ov_Reference_Owned " + name.get() + ";\n";
        }

        interface += "\n\n";
        implementation += "\n\n";
    }

    void Translator::write_functions(std::string& interface, std::string& implementation) {
        interface += "#include <include.h>\n";
        interface += "#include \"structures.h\"\n";
        interface += "\n\n";

        implementation += "#include \"functions.h\"\n";
        implementation += "\n\n";

        for (auto const& lambda : code.lambdas)
            implementation += "Ov_Reference_Owned lambda_" + std::to_string(lambda->get_id()) + "_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]);\n";
        for (auto const& [name, instructions] : code.imports)
            implementation += "void import_" + name + "_body();\n";

        implementation += "\n\n";

        for (auto const& lambda : code.lambdas) {
            implementation += "Ov_Reference_Owned lambda_" + std::to_string(lambda->get_id()) + "_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {\n";
            for (size_t i = 0; i < lambda->captures.size(); ++i)
                implementation += "\tOv_Reference_Shared " + lambda->captures[i].first.get() + " = captures[" + std::to_string(i) + "];\n";
            for (auto const& instruction : lambda->body.body)
                implementation += add_indentation(instruction->get_instruction_code());
            implementation += "\treturn " + lambda->body.return_value->get_expression_code() + ";\n";
            implementation += "}\n\n";
        }

        for (auto const& [name, main] : code.imports) {
            implementation += "void import_" + name + "_body() {\n";
            for (auto const& instruction : main.body)
                implementation += add_indentation(instruction->get_instruction_code());
            implementation += "}\n\n";
        }

        for (auto const& function : code.functions) {
            if (function->filter.return_value != nullptr) {
                interface += "bool " + function->name.get() + "_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]);\n";

                implementation += "bool " + function->name.get() + "_filter(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {\n";
                for (size_t i = 0; i < function->captures.size(); ++i) {
                    auto const& name = function->captures[i].first;
                    auto check = [&name](decltype(function->captures)::value_type const& tuple) -> bool {
                        return tuple.first == name;
                    };
                    if (std::find_if(function->parameters.begin(), function->parameters.end(), check) == function->parameters.end())
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
                    implementation += add_indentation(instruction->get_instruction_code());
                implementation += "\treturn " + function->filter.return_value->get_expression_code() + ";\n";

                implementation += "}\n";
            }

            {
                interface += "Ov_Reference_Owned " + function->name.get() + "_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]);\n";

                implementation += "Ov_Reference_Owned " + function->name.get() + "_body(Ov_Reference_Shared captures[], Ov_Reference_Shared args[], Ov_Reference_Shared local_variables[]) {\n";
                for (size_t i = 0; i < function->captures.size(); ++i) {
                    auto const& name = function->captures[i].first;
                    auto check = [&name](decltype(function->captures)::value_type const& tuple) -> bool {
                        return tuple.first == name;
                    };
                    if (std::find_if(function->parameters.begin(), function->parameters.end(), check) == function->parameters.end())
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
                    implementation += add_indentation(instruction->get_instruction_code());
                auto return_value = function->body.return_value;
                implementation += "\treturn " + (return_value->owned ? return_value->get_expression_code() : "Ov_Reference_copy(" + return_value->get_expression_code() + ")") + ";\n";

                implementation += "}\n\n";
            }
        }
    }

    void Translator::write_main(std::string& implementation) {
        implementation += "int main(int, char**) {\n";
        implementation += "\tOv_init();\n";

        for (auto const& [var, _] : code.global_variables) {
            if (!symbols.contains(var.symbol)) {
                implementation += "\tOv_UnknownData " + var.get() + "_data = { .vtable = &Ov_VirtualTable_Object, .data.ptr = Ov_GC_alloc_object(&Ov_VirtualTable_Object) };\n";
                implementation += "\t" + var.get() + " = Ov_Reference_new_symbol(" + var.get() + "_data);\n";
            }
        }

        for (auto const& instruction : code.main.body)
            implementation += add_indentation(instruction->get_instruction_code());

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
        return name;
    }

    std::string RawData::get_expression_code() const {
        if (std::holds_alternative<bool>(value->value))
            return "(Ov_Data){ .b = " + value->get_expression_code() + " }";
        if (std::holds_alternative<OV_INT>(value->value))
            return "(Ov_Data){ .i = " + value->get_expression_code() + " }";
        if (std::holds_alternative<OV_FLOAT>(value->value))
            return "(Ov_Data){ .f = " + value->get_expression_code() + " }";

        assert(false);
        return "(Ov_Data){ .ptr = NULL }";
    }

    std::string Affectation::get_expression_code() const {
        std::string code;

        if (auto ref = std::dynamic_pointer_cast<Reference>(lvalue)) {
            if (declare) {
                if (ref->owned)
                    code += "Ov_Reference_Owned ";
                else
                    code += "Ov_Reference_Shared ";
            }
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

    std::string If::get_instruction_code() const {
        std::string code;

        code += "if (" + condition->get_expression_code() + ") {\n";
        for (auto const& instruction : body)
            code += add_indentation(instruction->get_instruction_code());
        if (!alternative.empty()) {
            code += "} else {\n";
            for (auto const& instruction : alternative)
                code += add_indentation(instruction->get_instruction_code());
        }
        code += "}\n";

        return code;
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

        if (!objects.empty()) {
            code += "Ov_Reference_Shared array" + std::to_string(id) + "[] = { ";
            for (auto const& e : objects)
                code += "Ov_Reference_share(" + e->get_expression_code() + "), ";
            code += "};\n";
        }

        return code;
    }

    std::string FunctionExpression::get_expression_code() const {
        return "expression" + std::to_string(id);
    }
    std::string FunctionExpression::get_instruction_code() const {
        std::string code;

        if (auto const* tuple = std::get_if<std::vector<std::shared_ptr<FunctionExpression>>>(this)) {
            for (auto const& e : *tuple)
                code += e->get_instruction_code() + "\n";

            code += "Ov_Expression expression" + std::to_string(id) + "_array[] = { ";
            for (auto const& e : *tuple)
                code += e->get_expression_code() + ", ";
            code += "};\n";
            code += "Ov_Expression expression" + std::to_string(id) + " = { .type = Ov_EXPRESSION_TUPLE, .tuple.size = " + std::to_string(tuple->size()) + ", .tuple.tab = expression" + std::to_string(id) + "_array };";
        } else if (auto const* ref = std::get_if<std::shared_ptr<Reference>>(this)) {
            code += "Ov_Expression expression" + std::to_string(id) + " = { .type = Ov_EXPRESSION_REFERENCE, .reference = Ov_Reference_share(" + (*ref)->get_expression_code() + ") };";
        } else if (auto const* lambda = std::get_if<std::shared_ptr<Lambda>>(this)) {
            code += "Ov_Expression expression" + std::to_string(id) + " = { .type = Ov_EXPRESSION_LAMBDA, .lambda = NULL };\n";

            if (!(*lambda)->captures.empty()) {
                code += "Ov_Reference_Shared lambda_" + std::to_string((*lambda)->get_id()) + "_captures[] = { ";
                for (auto const& capture : (*lambda)->captures)
                    code += "Ov_Reference_share(" + capture.first.get() + "), ";
                code += "};\n";
            }

            code += "Ov_Function_push(&expression" + std::to_string(id) + ".lambda, \"\", lambda_" + std::to_string((*lambda)->get_id()) + "_body, NULL, 0, " + ((*lambda)->captures.empty() ? "NULL" : ("lambda_" + std::to_string((*lambda)->get_id()) + "_captures")) + ", " + std::to_string((*lambda)->captures.size()) + ");";
        }

        return code;
    }

}
