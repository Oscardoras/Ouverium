#pragma once

#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <variant>

#include "../../parser/Parser.hpp"

#include "../../Utils.hpp"


namespace Static {

    struct Function;
    struct Reference;
    struct Data {
        std::optional<bool> Bool;
        std::optional<char> Char;
        std::optional<INT> Int;
        std::optional<FLOAT> Float;

        std::optional<Function> function;
        std::unique_ptr<Data> array;
        std::map<std::string, Data> properties;

        std::set<std::weak_ptr<Reference>> assignators;
        std::set<Data*> get_linked() {
            std::set<Data*> linked = { this };

            for (auto const& r : assignators) {
                if (auto reference = r.lock()) {
                    auto l = reference->get_data().get_linked();
                    linked.insert(l.begin(), l.end());
                }
            }

            return linked;
        }
    };

    struct LambdaFunction {
        std::shared_ptr<Reference> value;
    };
    struct CustomFunction {
        std::shared_ptr<Parser::FunctionDefinition> function;
    };
    struct SystemFunction : public std::function<std::shared_ptr<Reference>(std::shared_ptr<Reference>)> {};
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

        virtual void iterate(std::set<std::shared_ptr<Reference>>& linked) {
            if (linked.find(shared_from_this()) == linked.end()) {
                linked.insert(shared_from_this());

                for (auto const& child : children)
                    if (auto c = child.lock())
                        c->iterate(linked);
            }
        }

        friend class ArrayReference;
        friend class PropertyReference;
        friend class ForwardedReference;
        friend class TupleReference;
        friend class Data;

    public:

        std::set<std::weak_ptr<ForwardedReference>> children;

        virtual Data& get_data() = 0;

        std::vector<Function> get_functions() {
            std::vector<Function> functions;

            for (auto const& l : get_linked())
                for (auto d : l->get_data().get_linked())
                    if (auto& f = d->function)
                        functions.push_back(*f);

            return functions;
        }

    };

    struct ForwardedReference : public Reference {

    protected:

        std::set<std::shared_ptr<Reference>> parents;

        void iterate(std::set<std::shared_ptr<Reference>>& linked) override {
            if (linked.find(shared_from_this()) == linked.end()) {
                linked.insert(shared_from_this());

                for (auto const& child : children)
                    if (auto c = child.lock())
                        c->iterate(linked);

                for (auto const& parent : parents)
                    parent->iterate(linked);
            }
        }

        friend class Reference;

    public:

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
        SymbolReference() :
            data{ std::make_shared<Data>() } {}
        Data& get_data() override {
            return *data;
        }
    };
    struct PropertyReference : public Reference {
        std::shared_ptr<Reference> parent;
        std::string name;
        PropertyReference(std::shared_ptr<Reference> parent, std::string name) :
            parent{ parent }, name{ name } {}
        Data& get_data() override {
            // TODO
            return parent->get_data().properties[name];
        }
    };
    struct ArrayReference : public Reference {
        std::shared_ptr<Reference> array;
        ArrayReference(std::shared_ptr<Reference> const& array, size_t) :
            array{ array } {}
        Data& get_data() override {
            if (array->get_data().array == nullptr)
                array->get_data().array = std::make_unique<Data>();

            return *array->get_data().array;
        }
    };
    struct TupleReference : public Reference, public std::vector<std::shared_ptr<Reference>> {
        Data data;
        TupleReference(std::vector<std::shared_ptr<Reference>> const& vector) :
            std::vector<std::shared_ptr<Reference>>{ vector } {
            for (auto const& r : vector)
                data.assignators.insert(r);
        }
        Data& get_data() override {
            return data;
        }
    };

}
