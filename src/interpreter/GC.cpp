#include <mutex>

#include "Interpreter.hpp"


namespace Interpreter {

    namespace GC {

        std::mutex mutex;
        std::list<ObjectPtr::type> objects;
        size_t last_size = 1024;
        std::list<SymbolReference::type> references;
        std::set<Context*> contexts;

        void add_context(Context& context) {
            std::lock_guard guard(mutex);
            contexts.insert(&context);
        }

        void remove_context(Context& context) {
            std::lock_guard guard(mutex);
            contexts.erase(&context);
        }

        ObjectPtr new_object(Object const& object) {
            if (objects.size() >= 2 * last_size)
                collect();

            std::lock_guard guard(mutex);
            objects.push_back({ object, false });
            return --objects.end();
        }

        SymbolReference new_reference(Data const& data) {
            std::lock_guard guard(mutex);
            references.push_back({ data, false });
            return { --references.end() };
        }

        void collect() {
            auto objects_end = objects.end();
            auto references_end = references.end();

            std::vector<std::reference_wrapper<const Data>> grey;
            std::set<Data*> symbol_references;
            {
                std::lock_guard guard(mutex);
                auto add_grey = [&grey, &symbol_references](Interpreter::IndirectReference const& ref) {
                    if (auto symbol_reference = std::get_if<SymbolReference>(&ref)) {
                        grey.push_back(symbol_reference->it->first);
                        symbol_reference->it->second = true;
                    } else if (auto property_reference = std::get_if<PropertyReference>(&ref)) {
                        grey.push_back(property_reference->parent);
                    } else if (auto array_reference = std::get_if<ArrayReference>(&ref)) {
                        grey.push_back(array_reference->array);
                    }
                };

                for (Context* context : contexts) {
                    if (auto global = dynamic_cast<GlobalContext*>(context))
                        grey.push_back(global->system);

                    for (auto const& [_, ref] : *context)
                        add_grey(ref);
                }

                while (!grey.empty()) {
                    Data const& data = grey.back();
                    grey.pop_back();

                    if (auto obj = get_if<ObjectPtr>(&data); obj) {
                        auto const& object = *obj;

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

            }
            for (auto it = objects.begin(); it != objects_end;) {
                if (!it->second) {
                    it = objects.erase(it);
                } else {
                    it->second = false;
                    ++it;
                }
            }
            for (auto it = references.begin(); it != references_end;) {
                if (!it->second) {
                    it = references.erase(it);
                } else {
                    it->second = false;
                    ++it;
                }
            }

            last_size = objects.size();
        }
    }


    ObjectPtr::ObjectPtr(ObjectPtr const& ptr) {
        it = ptr.it;
    }

    ObjectPtr& ObjectPtr::operator=(ObjectPtr const& ptr) {
        std::lock_guard guard(GC::mutex);

        it = ptr.it;
        return *this;
    }

    ObjectPtr::~ObjectPtr() {
        std::lock_guard guard(GC::mutex);
    }

    Object& ObjectPtr::operator*() const {
        return it->first;
    }

    Object* ObjectPtr::operator->() const {
        return &it->first;
    }

    bool operator==(ObjectPtr const& a, ObjectPtr const& b) {
        return &(*a) == &(*b);
    }

    bool operator!=(ObjectPtr const& a, ObjectPtr const& b) {
        return !(a == b);
    }

}
