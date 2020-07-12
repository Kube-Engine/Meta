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

        template<typename Type>
        class Factory;
        class Resolver;
    }
}