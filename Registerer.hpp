/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Registerer
 */

#pragma once

#include <Kube/Core/TrivialDispatcher.hpp>

#include "Meta.hpp"

namespace kF::Meta
{
    class Registerer;

    template<auto Functor>
    struct RegisterLater;

    template<auto Functor>
    struct RegisterTemplateLater;
}

/** @brief Registerer is used to store types that need their meta data to be registered */
class kF::Meta::Registerer
{
public:
    /** @brief Register every type registered with 'RegisterLater' methods */
    static void RegisterMetadata(void)
        { _Dispatcher.dispatch(); _TemplateDispatcher.dispatch(); }


    /** @brief Store a functor to be called at class registration time */
    template<auto Functor>
    static void RegisterLater(void) { _Dispatcher.add(Functor); }

    /** @brief Store a functor to be called at template registration time */
    template<auto Functor>
    static void RegisterTemplateLater(void) { _TemplateDispatcher.add(Functor); }

private:
    static inline Core::TrivialDispatcher<void(void)> _Dispatcher {};
    static inline Core::TrivialDispatcher<void(void)> _TemplateDispatcher {};

    /** @brief Registerer is a singleton */
    Registerer(void);
    ~Registerer(void);
};

/** @brief Helper used to register a class later in registerer */
template<auto Functor>
struct kF::Meta::RegisterLater
{
    RegisterLater(void) { Registerer::RegisterLater<Functor>(); }
};

/** @brief Helper used to register a template class later in registerer */
template<auto Functor>
struct kF::Meta::RegisterTemplateLater
{
    RegisterTemplateLater(void) { Registerer::RegisterTemplateLater<Functor>(); }
};