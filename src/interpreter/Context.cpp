#include <mutex>

#include "Interpreter.hpp"


namespace Interpreter {

    static std::mutex new_object_mutex;
    static std::mutex new_reference_mutex;

    Object* Context::new_object(Object const& object) {
        auto& global = get_global();
        auto& objects = global.objects;
        // if (objects.size() >= 2 * global.last_size) {
        //     global.GC_collect();
        // }
        std::lock_guard guard(new_object_mutex);
        objects.push_back(std::move(object));
        return &objects.back();
    }

    Data& Context::new_reference(Data const& data) {
        auto& references = get_global().references;
        std::lock_guard guard(new_reference_mutex);
        references.push_back(data);
        return references.back();
    }

    std::set<std::string> Context::get_symbols() const {
        std::set<std::string> symbols;
        for (auto const& symbol : *this)
            symbols.insert(symbol.first);
        return symbols;
    }

    bool Context::has_symbol(std::string const& symbol) const {
        return symbols.find(symbol) != symbols.end();
    }

    void Context::add_symbol(std::string const& symbol, IndirectReference const& indirect_reference) {
        symbols.emplace(symbol, indirect_reference);
    }

    void Context::add_symbol(std::string const& symbol, Data const& data) {
        symbols.emplace(symbol, new_reference(data));
    }

    IndirectReference Context::operator[](std::string const& symbol) {
        auto it = symbols.find(symbol);
        if (it == symbols.end())
            return symbols.emplace(symbol, new_reference()).first->second;
        else
            return it->second;
    }


    GlobalContext::GlobalContext(std::shared_ptr<Parser::Expression> expression) :
        Context(expression) {
        contexts.insert(this);
        system = new_object();
        SystemFunctions::init(*this);
    }

    void GlobalContext::GC_collect() {
        std::vector<Data> grey;
        std::set<Data*> symbol_references;
        auto add_grey = [&grey, &symbol_references](Interpreter::IndirectReference const& ref) {
            if (auto symbol_reference = std::get_if<SymbolReference>(&ref)) {
                grey.push_back(symbol_reference->get());
                symbol_references.insert(&symbol_reference->get());
            } else if (auto property_reference = std::get_if<PropertyReference>(&ref)) {
                grey.push_back(property_reference->parent);
            } else if (auto array_reference = std::get_if<ArrayReference>(&ref)) {
                grey.push_back(array_reference->array);
            }
        };

        grey.push_back(Data(system));
        for (Context* context : contexts)
            for (auto const& [_, ref] : *context)
                add_grey(ref);

        while (!grey.empty()) {
            Data data = grey.back();
            grey.pop_back();

            if (auto obj = get_if<Object*>(&data); obj) {
                Object* object = *obj;

                if (!object->referenced) {
                    object->referenced = true;

                    for (auto const& [sqfq, d] : object->properties)
                        grey.push_back(d);
                    for (auto const& d : object->array)
                        grey.push_back(d);
                    for (auto const& f : object->functions)
                        for (auto const& [_, capture] : f.extern_symbols)
                            add_grey(capture);
                }
            }

        }

        for (auto it = objects.begin(); it != objects.end();) {
            if (!it->referenced) {
                it = objects.erase(it);
            } else {
                it->referenced = false;
                ++it;
            }
        }
        for (auto it = references.begin(); it != references.end();) {
            if (!symbol_references.contains(&(*it))) {
                it = references.erase(it);
            } else {
                ++it;
            }
        }

        last_size = objects.size();
    }

}
