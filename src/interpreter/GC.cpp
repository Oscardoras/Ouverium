#include <cassert>
#include <mutex>

#include "Data.hpp"
#include "Interpreter.hpp"


namespace Interpreter {

    namespace GC {

        std::mutex mutex;
        std::list<ObjectPtr::type> objects;
        size_t last_size = 1024;
        std::list<SymbolReference::type> references;
        std::set<Context*> contexts;

        void add_context(Context& context) {
            std::lock_guard lock(mutex);
            contexts.insert(&context);
        }

        void remove_context(Context& context) {
          std::lock_guard lock(mutex);
          contexts.erase(&context);
        }

        ObjectPtr new_object(Object const& object) {
            if (objects.size() >= 2 * last_size)
                collect();

            std::lock_guard lock(mutex);
            objects.emplace_back(object, 0);
            return --objects.end();
        }

        SymbolReference new_reference(Data const& data) {
          std::lock_guard lock(mutex);
          references.emplace_back(data, 0);
          return {--references.end()};
        }

        void collect() {
            auto objects_end = objects.end();
            auto references_end = references.end();

            std::vector<std::reference_wrapper<const Data>> grey;
            {
              std::lock_guard lock(mutex);
              auto add_grey =
                  [&grey](Interpreter::IndirectReference const &ref) {
                    if (auto const* symbol_reference = std::get_if<SymbolReference>(&ref)) {
                      grey.emplace_back(symbol_reference->it->first);
                      symbol_reference->it->second = true;
                    } else if (auto const* property_reference = std::get_if<PropertyReference>(&ref)) {
                      grey.emplace_back(property_reference->parent);
                    } else if (auto const* array_reference = std::get_if<ArrayReference>(&ref)) {
                      grey.emplace_back(array_reference->array);
                    }
                  };

              for (Context *context : contexts) {
                if (auto *global = dynamic_cast<GlobalContext *>(context))
                  grey.emplace_back(global->system);

                for (auto const &[_, ref] : *context)
                  add_grey(ref);
                }

                while (!grey.empty()) {
                    Data const& data = grey.back();

                    if (auto const* obj = get_if<ObjectPtr>(&data); obj) {
                        auto const& object = *obj;
                        grey.pop_back();

                        if (!object.it->second) {
                            object.it->second = true;

                            for (auto const& [sqfq, d] : object->properties)
                                grey.emplace_back(d);
                            for (auto const& d : object->array)
                                grey.emplace_back(d);
                            for (auto const& f : object->functions)
                                for (auto const& [_, capture] : f.extern_symbols)
                                    add_grey(capture);
                        }
                    } else {
                        grey.pop_back();
                    }
                }

            }
            for (auto it = objects.begin(); it != objects_end;) {
                if (!it->second) {
                    it->second = -1;
                    it = objects.erase(it);
                    // ++it;
                } else {
                    it->second = false;
                    ++it;
                }
            }
            for (auto it = references.begin(); it != references_end;) {
                if (!it->second) {
                    it->second = -1;
                    assert(it->second == -1);
                    it = references.erase(it);
                    // ++it;
                } else {
                    it->second = false;
                    ++it;
                }
            }

            last_size = objects.size();
        }
    }


    ObjectPtr::ObjectPtr(ObjectPtr const& ptr) :
        it(ptr.it) {
        assert(it->second != -1);
    }

    ObjectPtr::ObjectPtr(ObjectPtr&& ptr) noexcept :
        it(ptr.it) {
        assert(it->second != -1);
    }

    ObjectPtr& ObjectPtr::operator=(ObjectPtr const& ptr) {
        ObjectPtr(ptr).swap(*this);

        return *this;
    }

    ObjectPtr& ObjectPtr::operator=(ObjectPtr&& ptr) noexcept {
        ObjectPtr(ptr).swap(*this);

        return *this;
    }

    ObjectPtr::~ObjectPtr() { std::lock_guard lock(GC::mutex); }

    Object& ObjectPtr::operator*() const {
        assert(it->second != -1);

        return it->first;
    }

    Object* ObjectPtr::operator->() const {
        assert(it->second != -1);

        return &it->first;
    }

    bool operator==(ObjectPtr const& a, ObjectPtr const& b) {
        return &(*a) == &(*b);
    }

    bool operator!=(ObjectPtr const& a, ObjectPtr const& b) {
        return !(a == b);
    }

    void ObjectPtr::swap(ObjectPtr& ptr) {
        std::swap(it, ptr.it);
    }

}
