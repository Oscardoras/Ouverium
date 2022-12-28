#include "Functions.hpp"


namespace Analyzer {

    namespace Functions {

        M<Reference> separator(Context & context) {
            return context["b"];
        }

        M<Reference> if_statement(Context & context) {
            M<Reference> m;
            for (auto f : context["function"]) {
                auto function = std::get<std::shared_ptr<FunctionDefinition>>((*std::get<Object*>(f.get())->functions.begin())->ptr)->body;

                if (auto tuple = std::dynamic_pointer_cast<Tuple>(function)) {
                    if (tuple->objects.size() >= 2) {
                        auto & parent = context.get_parent();

                        auto first_condition = execute(parent, tuple->objects[0]).to_object(context);
                        if (auto c = std::get_if<bool*>(&first_condition->data)) {
                            if (!first_condition->accurate || *c)
                                return execute(parent, tuple->objects[1]);
                            if (!first_condition->accurate || !*c) {
                                unsigned long i = 2;
                                while (i < tuple->objects.size()) {
                                    auto else_s = execute(parent, tuple->objects[i]).to_object(context);
                                    if (else_s == context["else"] && i+1 < tuple->objects.size()) {
                                        auto s = execute(parent, tuple->objects[i+1]).to_object(context);
                                        if (s == context["if"]) {
                                            if (i+3 < tuple->objects.size()) {
                                                auto condition = execute(parent, tuple->objects[i+2]).to_object(context);
                                                if (c = std::get_if<bool*>(&condition->data)) {
                                                    if (!condition->accurate || *c)
                                                        return execute(parent, tuple->objects[i+3]);
                                                    if (!condition->accurate || !*c) i += 4;
                                                } else throw FunctionArgumentsError();
                                            } else throw FunctionArgumentsError();
                                        } else return s;
                                    } else throw FunctionArgumentsError();
                                }
                                return context.new_object();
                            }
                        } else throw FunctionArgumentsError();
                    } else throw FunctionArgumentsError();
                } else throw FunctionArgumentsError();
            }
        }

    }

}
