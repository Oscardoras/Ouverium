#pragma once

#include <map>
#include <optional>
#include <set>
#include <string>
#include <variant>

#include "../../parser/Parser.hpp"

#include "../../Utils.hpp"


namespace Test {

    struct Function;
    struct Data {
        /*
        std::optional<bool> Bool;
        std::optional<char> Char;
        std::optional<long> Int;
        std::optional<double> Float;
        */

        std::optional<Function> function;
    };

    struct LambdaFunction {
        std::shared_ptr<Reference> value;
    };
    struct CustomFunction {
        std::shared_ptr<Parser::FunctionDefinition> function;
    };
    struct SystemFunction {};
    struct Function : public std::variant<LambdaFunction, CustomFunction, SystemFunction> {
        using std::variant<LambdaFunction, CustomFunction, SystemFunction>::variant;
    };

    class Reference : public std::enable_shared_from_this<Reference> {

    protected:

        std::set<std::shared_ptr<Reference>> get_linked() {
            std::set<std::shared_ptr<Reference>> linked;
            iterate(linked);
            return linked;
        }

    public:

        std::set<std::weak_ptr<ForwardedReference>> children;

        virtual void iterate(std::set<std::shared_ptr<Reference>> & linked) {
            if (linked.find(shared_from_this()) == linked.end()) {
                linked.insert(shared_from_this());

                for (auto const& child : children)
                    if (auto c = child.lock())
                        c->iterate(linked);
            }
        }

        virtual Data& get_data() = 0;

        std::vector<Function> const& get_functions() {
            std::vector<Function> functions;

            for (auto const& l : get_linked())
                if (auto & f = l->get_data().function)
                    functions.push_back(*f);

            return functions;
        }

    };

    struct ForwardedReference : public Reference {

    protected:

        std::set<std::weak_ptr<Reference>> parents;

    public:

        void iterate(std::set<std::shared_ptr<Reference>> & linked) override {
            if (linked.find(shared_from_this()) == linked.end()) {
                linked.insert(shared_from_this());

                for (auto const& child : children)
                    if (auto c = child.lock())
                        c->iterate(linked);

                for (auto const& parent : parents)
                    if (auto p = parent.lock())
                        p->iterate(linked);
            }
        }

        void link(std::shared_ptr<Reference> parent) {
            parents.insert(parent);
            parent->children.insert(std::static_pointer_cast<ForwardedReference>(shared_from_this()));
        }

    };
    struct DirectReference : public Reference, public Data {
        using Data::Data;
        Data& get_data() override {
            return *this;
        }
    };
    struct SymbolReference : public Reference {
        std::shared_ptr<Data> data;
        Data& get_data() override {
            return *data;
        }
    };
    struct PropertyReference : public Reference {
        std::shared_ptr<Reference> parent;
        std::string name;
        PropertyReference(std::shared_ptr<Reference> parent, std::string name):
            parent{ parent }, name{ name } {}
        Data& get_data() override {
            // TODO
        }
    };
    struct ArrayReference : public Reference {
        std::shared_ptr<Reference> array;
        Data& get_data() override {
            // TODO
        }
    };
    struct TupleReference : public Reference, public std::vector<std::shared_ptr<Reference>> {
        using std::vector<std::shared_ptr<Reference>>::vector;
        Data& get_data() override {
            // TODO
        }
    };

}
