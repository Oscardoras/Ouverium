#include "Functions.hpp"


namespace Analyzer {

    namespace Functions {

        inline auto get_function(Analyzer::M<std::reference_wrapper<Analyzer::Data>> reference) {
            return std::get<Object*>(reference.begin()->get())->functions;
        }

        M<Reference> separator(Context & context) {
            return context["b"];
        }

        void if_function(M<Reference> & m, Context & context, std::shared_ptr<Tuple> const& tuple, size_t i) {
            auto & parent = context.get_parent();

            auto conditions = to_data(execute(parent, tuple->objects[i]), context);
            for (auto condition : conditions) {
                if (auto c = std::get_if<bool>(&condition)) {
                    if (!condition.defined || *c) {
                        auto r = execute(parent, tuple->objects[i+1]);
                        m.insert(m.end(), r.begin(), r.end());
                    }
                    if (!condition.defined || !*c) {
                        if (i+3 < tuple->objects.size()) {
                            auto else_s = to_data(execute(parent, tuple->objects[i+2]), context);
                            if (M<std::reference_wrapper<Data>>(else_s) == context["else"]) {
                                auto s = to_data(execute(parent, tuple->objects[i+3]), context);
                                if (i+4 == tuple->objects.size())
                                    m.insert(m.end(), s.begin(), s.end());
                                else if (i+6 < tuple->objects.size() && M<std::reference_wrapper<Data>>(s) == context["if"])
                                    if_function(m, context, tuple, i+4);
                                else throw FunctionArgumentsError();
                            } else throw FunctionArgumentsError();
                        } else m.push_back(Reference(context.new_object()));
                    }
                } else throw FunctionArgumentsError();
            }
        }

        M<Reference> if_statement(Context & context) {
            auto function = std::get<std::shared_ptr<FunctionDefinition>>((*get_function(context["function"]).begin())->ptr)->body;
            if (auto tuple = std::dynamic_pointer_cast<Tuple>(function)) {
                if (tuple->objects.size() >= 3) {
                    M<Reference> m;
                    if_function(m, context, tuple, 0);
                    return m;
                } else throw FunctionArgumentsError();
            } else throw FunctionArgumentsError();
        }

        M<Reference> while_statement(Context & context) {
            M<Reference> m;

            auto condition = get_function(context["condition"]);
            auto block = get_function(context["block"]);
            while (true) {
                auto c = to_data(call_function(context.get_parent(), nullptr, condition, std::make_shared<Tuple>()), context);
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

                if (status == UNKNOWN || status == TRUE || status == UNDEFINED)
                    m = call_function(context.get_parent(), nullptr, block, std::make_shared<Tuple>());
                if (status == FALSE || status == UNDEFINED)
                    break;
            }

            if (m.empty())
                m.push_back(Data(context.new_object()));
            return m;
        }

    }

}
