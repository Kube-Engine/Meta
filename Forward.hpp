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
        template<typename RegisteredType>
        class Factory;
        class Resolver;
    }
}