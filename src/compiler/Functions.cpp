#include "Functions.hpp"


namespace Analyzer {

    namespace Functions {

        inline auto get_function(std::reference_wrapper<M<Data>> reference) {
            return std::get<Object*>(*reference.get().begin())->functions;
        }

        M<Reference> separator(Context & context, bool potential) {
            return Reference(context["b"]);
        }

        void if_function(M<Reference> & m, Context & context, bool potential, std::shared_ptr<Tuple> const& tuple, size_t i) {
            auto & parent = context.get_parent();

            auto conditions = to_data(execute(parent, potential, tuple->objects[i]), context);
            bool TRUE = false;
            bool FALSE = false;
            bool defined = true;
            for (auto condition : conditions) {
                if (auto c = std::get_if<bool>(&condition)) {
                    if (!condition.defined || *c)
                        TRUE = true;
                    if (!condition.defined || !*c)
                        FALSE = true;
                    if (!condition.defined)
                        defined = false;
                    if (TRUE && FALSE)
                        break;
                } else throw FunctionArgumentsError();
            }

            if (TRUE) {
                auto r = execute(parent, potential || !defined, tuple->objects[i+1]);
                m.insert(m.end(), r.begin(), r.end());
            }
            if (FALSE) {
                if (i+3 < tuple->objects.size()) {
                    auto else_s = to_data(execute(parent, potential || !defined, tuple->objects[i+2]), context);
                    if (else_s == context["else"].get()) {
                        auto s = to_data(execute(parent, potential || !defined, tuple->objects[i+3]), context);
                        if (i+4 == tuple->objects.size())
                            m.insert(m.end(), s.begin(), s.end());
                        else if (i+6 < tuple->objects.size() && s == context["if"].get())
                            if_function(m, context, potential || !defined, tuple, i+4);
                        else throw FunctionArgumentsError();
                    } else throw FunctionArgumentsError();
                } else m.push_back(M<Data>(context.new_object()));
            }
        }

        M<Reference> if_statement(Context & context, bool potential) {
            auto function = std::get<std::shared_ptr<FunctionDefinition>>((*get_function(context["function"]).begin())->ptr)->body;
            if (auto tuple = std::dynamic_pointer_cast<Tuple>(function)) {
                if (tuple->objects.size() >= 3) {
                    M<Reference> m;
                    if_function(m, context, potential, tuple, 0);
                    return m;
                } else throw FunctionArgumentsError();
            } else throw FunctionArgumentsError();
        }

        M<Reference> while_statement(Context & context, bool potential) {
            M<Reference> m;

            auto condition = get_function(context["condition"]);
            auto block = get_function(context["block"]);
            while (true) {
                auto c = to_data(call_function(context.get_parent(), potential, nullptr, condition, std::make_shared<Tuple>()), context);
                enum { UNKNOWN, UNDEFINED, TRUE, FALSE } status;
                for (auto d : c) {
                    if (auto b = std::get_if<bool>(&d)) {
                        if (!d.defined) {
                            status = UNDEFINED;
                            break;
                        } else {
                            if (status == UNKNOWN) {
                                if (b) status = TRUE;
                                else status = FALSE;
                            } else if (status == TRUE && !b || status == FALSE && b) {
                                status = UNDEFINED;
                                break;
                            }
                        }
                    } else throw FunctionArgumentsError();
                }
                if (status == UNDEFINED)
                    potential = true;

                if (status == UNKNOWN || status == TRUE || status == UNDEFINED)
                    m = call_function(context.get_parent(), potential, nullptr, block, std::make_shared<Tuple>());
                if (status == FALSE || status == UNDEFINED)
                    break;
            }

            if (m.empty())
                m.push_back(M<Data>(context.new_object()));
            return m;
        }

        void assignation(M<Reference> const& var, M<Data> const& data, bool potential) {
            for (auto reference : var) {
                if (auto ref = std::get_if<std::reference_wrapper<M<Data>>>(&reference))
                    if (potential)
                        ref->get().insert(ref->get().end(), data.begin(), data.end());
                    else
                        ref->get() = data;
                else if (auto tuple = std::get_if<std::vector<M<Reference>>>(&reference))
                    for (auto d : data)
                        if (auto object = std::get_if<Object*>(&d)) {
                            for (long i = 0; i < tuple->size(); i++)
                                assignation((*tuple)[i], (*object)->array[i+1], potential);
                        } else throw FunctionArgumentsError();
            }
        }

        M<Reference> assign(Context & context, bool potential) {
            auto var = call_function(context, potential, nullptr, get_function(context["var"]), std::make_shared<Tuple>());
            auto object = to_data(M<Reference>(context["object"]), context);

            assignation(var, object, potential);
            return var;
        }

        M<Reference> function_definition(Context & context, bool potential) {
            auto var = context["var"];
            auto object = context["object"];

            for (auto it = object->functions.rbegin(); it != object->functions.rend(); it++) {
                if ((*it)->type == Function::Custom) {
                    var->functions.push_front(std::make_unique<CustomFunction>((CustomFunction&) **it));
                } else var->functions.push_front(std::make_unique<SystemFunction>((SystemFunction&) **it));
            }

            return context.get_symbol("var");
        }

    }

}
