/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Forward all meta classes
 */

#pragma once

namespace kF
{
    class Var;

    namespace Meta
    {
        class Type;
        class Constructor;
        class Converter;
        class Function;
        class Data;
        class SlotTable;
        class Signal;
        class Resolver;

        template<typename RegisteredType>
        class FactoryBase;

        template<typename RegisteredType>
        using Factory = FactoryBase<std::remove_cvref_t<RegisteredType>>;
    }
}