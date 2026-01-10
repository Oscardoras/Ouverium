#pragma once

#include "StaticAnalyzer.hpp"


namespace Static {

    std::shared_ptr<Reference> separator(std::shared_ptr<Reference> reference) {
        if (auto tuple = std::dynamic_pointer_cast<TupleReference>(reference)) {
            if (tuple->empty())
                return std::make_shared<TupleReference>();
            else
                return tuple->back();
        } else {
            return reference;
        }
    }

    std::shared_ptr<Reference> setter(std::shared_ptr<Reference> reference) {
        if (auto tuple = std::dynamic_pointer_cast<TupleReference>(reference)) {
            if (tuple->size() == 2) {
                tuple->at(0)->get_data().assignators.insert(tuple->at(1));
                return tuple->at(0);
            }
        }

        return nullptr;
    }

}
