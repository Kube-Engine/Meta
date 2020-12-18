/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Registerer
 */

#pragma once

#include <Kube/Core/TrivialDispatcher.hpp>

#include "Meta.hpp"
#include "TemplateDecomposer.hpp"

namespace kF::Meta
{
    class Registerer;
    struct RegisterLater;
    struct RegisterTemplateLater;
}

/** @brief Registerer is used to store types that need their meta data to be registered */
class kF::Meta::Registerer
{
public:
    struct alignas_cacheline Cache
    {
        Core::Vector<Core::TrivialFunctor<void(void)>> types;
        Core::Vector<Core::TrivialFunctor<void(void)>> templates;
    };

    /** @brief Register every type registered with 'RegisterLater' methods */
    static void RegisterMetadata(void)
    {
        for (auto &func : _Cache.types)
            func();
        for (auto &func : _Cache.templates)
            func();
    }

    /** @brief Store a functor to be called at class registration time */
    template<typename Type, typename Functor>
    static void RegisterLater(Functor &&functor)
    {
        using Decomposer = Internal::TemplateDecomposer<Type>;

        if constexpr (!Decomposer::IsTemplate)
            _Cache.types.push(std::forward<Functor>(functor));
        else
            _Cache.templates.push(std::forward<Functor>(functor));
    }

private:
    static inline Cache _Cache;

    /** @brief Registerer is a singleton */
    Registerer(void);
    ~Registerer(void);
};

/** @brief Helper used to register a class later in registerer */
class kF::Meta::RegisterLater
{
public:
    template<typename Type, typename Functor>
    [[nodiscard]] static RegisterLater Make(Functor &&functor) noexcept
    {
        Registerer::RegisterLater<Type>(std::forward<Functor>(functor));
        return RegisterLater();
    }
};