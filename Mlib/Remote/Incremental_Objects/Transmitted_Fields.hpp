#pragma once
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <cstdint>

namespace Mlib {

enum class TransmittedFields: TransmittedFieldsType {
    NONE = 0,
    SITE_ID = 1 << 0,
    END = 1 << 1
};

inline bool any(TransmittedFields tasks) {
    return tasks != TransmittedFields::NONE;
}

inline TransmittedFields operator & (TransmittedFields a, TransmittedFields b) {
    return (TransmittedFields)((TransmittedFieldsType)a & (TransmittedFieldsType)b);
}

inline TransmittedFields operator | (TransmittedFields a, TransmittedFields b) {
    return (TransmittedFields)((TransmittedFieldsType)a | (TransmittedFieldsType)b);
}

inline TransmittedFields& operator |= (TransmittedFields& a, TransmittedFields b) {
    (TransmittedFieldsType&)a |= (TransmittedFieldsType)b;
    return a;
}

inline TransmittedFields operator ~ (TransmittedFields a) {
    return (TransmittedFields)(~(TransmittedFieldsType)a);
}

}
