/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Type
 */

#include "Meta.hpp"

using namespace kF;

Var Meta::Type::defaultConstruct(void) const
{
    return (*_desc->defaultConstructFunc)();
}

Var Meta::Type::copyConstruct(void *data) const
{
    return (*_desc->copyConstructFunc)(data);
}

Var Meta::Type::moveConstruct(void *data) const
{
    return (*_desc->moveConstructFunc)(data);
}

Meta::Type Meta::Type::findBase(const Meta::Type type) const noexcept
{
    for (const auto &base : _desc->bases) {
        if (base == type)
            return base;
    }
    for (auto base : _desc->bases) {
        if (base = base.findBase(type); base)
            return base;
    }
    return Type();
}

Meta::Converter Meta::Type::findConverter(const Meta::Type type) const noexcept
{
    for (const auto &conv : _desc->converters)
        if (conv.convertType() == type)
            return conv;
    return Converter();
}

Meta::Function Meta::Type::findFunction(const HashedName name) const noexcept
{
    for (const auto &func : _desc->functions)
        if (func.name() == name)
            return func;
    for (Function res; const auto &base : _desc->bases)
        if (res = base.findFunction(name); res)
            return res;
    return Function();
}

Meta::Data Meta::Type::findData(const HashedName name) const noexcept
{
    for (const auto &data : _desc->datas)
        if (data.name() == name)
            return data;
    for (Data res; const auto &base : _desc->bases)
        if (res = base.findData(name); res)
            return res;
    return Data();
}

Meta::Signal Meta::Type::findSignal(const HashedName name) const noexcept
{
    for (const auto &signal : _desc->signals)
        if (signal.name() == name)
            return signal;
    for (Signal res; const auto &base : _desc->bases)
        if (res = base.findSignal(name); res)
            return res;
    return Signal();
}

Meta::Constructor Meta::Type::findConstructor(const std::vector<Type> &types) const noexcept
{
    Constructor preferred {};
    auto bestScore = 0u;

    for (const auto ctor : _desc->constructors) {
        auto i = 0u, score = 0u;
        if (types.size() != ctor.argsCount())
            continue;
        for (const auto type : types) {
            auto expectedType = ctor.argType(i);
            if (type == expectedType)
                ++score;
            else if (!type.findConverter(expectedType))
                break;
            ++i;
        }
        // We got a perfect match, return it
        if (score == types.size())
            return ctor;
        // We got a match but it requires conversion, store it but continue to check if there is a better match
        else if (i == types.size() && bestScore < score) {
            bestScore = score;
            preferred = ctor;
        }
    }
    return preferred;
}

void Meta::Type::clear(void)
{
    _desc->name = 0;
    _desc->bases.clear();
    _desc->constructors.clear();
    _desc->converters.clear();
    _desc->functions.clear();
    _desc->datas.clear();
    _desc->signals.clear();
}