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
        class Signal;
        class Slot;
        class DelayedSlot;
        class Connection;

        template<typename Type>
        class Factory;
        class Resolver;
    }
}