#include "Interpreter.hpp"


namespace Interpreter {

    void GlobalContext::GC_collect() {
        std::lock_guard guard(mutex);

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

            if (auto obj = get_if<ObjectPtr>(&data); obj) {
                ObjectPtr object = *obj;

                if (!object.it->second) {
                    object.it->second = true;

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
            if (!it->second) {
                it = objects.erase(it);
            } else {
                it->second = false;
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

    void ObjectPtr::affect() const {
        
    }

}
