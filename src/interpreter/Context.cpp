#include <mutex>

#include "Interpreter.hpp"


namespace Interpreter {

    static std::mutex new_object_mutex;
    static std::mutex new_reference_mutex;

    Object* Context::new_object(Object const& object) {
        auto& objects = get_global().objects;
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

    void Context::GC_collect() {
        // std::vector<Data> grey;

        // Context* context = this;
        // Context* old = nullptr;
        // while (context != old) {
        //     for (auto const& [_, ref] : *context) {
        //         grey.push_back(std::visit([](auto const& arg) -> Data& {
        //             return arg;
        //         }, ref));
        //     }

        //     old = context;
        //     context = &context->get_parent();
        // }

        // while (!grey.empty()) {
        //     Data const& data = grey.back();
        //     grey.pop_back();

        //     if (auto obj = get_if<Object*>(&data); obj && *obj) {
        //         Object* object = *obj;

        //         if (!object->referenced) {
        //             object->referenced = true;

        //             for (auto const& [_, d] : object->properties)
        //                 grey.push_back(d);
        //             for (auto const& d : object->array)
        //                 grey.push_back(d);
        //             for (auto const& f : object->functions) {
        //                 for (auto const& [_, capture] : f.extern_symbols) {
        //                     grey.push_back(std::visit([](auto const& arg) -> Data& {
        //                         return arg;
        //                     }, capture));
        //                 }
        //             }
        //         }
        //     }
        // }

        // GlobalContext& global = get_global();
        // for (auto it = global.objects.begin(); it != global.objects.end();) {
        //     if (!it->referenced) {
        //         it->destruct(*this);
        //         global.objects.erase(it++);
        //     } else {
        //         it->referenced = false;
        //         ++it;
        //     }
        // }
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
        system = new_object();
        SystemFunctions::init(*this);
    }

    GlobalContext::~GlobalContext() {
        for (auto& object : objects)
            object.destruct(*this);
    }

}
