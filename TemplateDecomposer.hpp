/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: TemplateDecomposer
 */

#pragma once

#include <tuple>

namespace kF::Meta::Internal
{
    template<typename>
    struct TemplateDecomposer;

    /** @brief Deduce a non template type */
    template<typename Type>
    struct TemplateDecomposer
    {
        static constexpr bool IsTemplate = false;
        static constexpr bool HasTypes = false;
        static constexpr bool HasVariables = false;
        static constexpr bool TypesBeforeArguments = false;
    };


    /** @brief Deduce a variadic list of template types */
    template<template<typename ...> typename Class, typename ...Types>
    struct TemplateDecomposer<Class<Types...>>
    {
        static constexpr bool IsTemplate = true;
        static constexpr bool HasTypes = true;
        static constexpr bool HasVariables = false;
        static constexpr bool TypesBeforeArguments = true;

        using TypesTuple = std::tuple<Types...>;
    };

    /** @brief Deduce a variadic list of template arguments */
    template<template<auto ...> typename Class, auto ...Vars>
    struct TemplateDecomposer<Class<Vars...>>
    {
        static constexpr bool IsTemplate = true;
        static constexpr bool HasTypes = false;
        static constexpr bool HasVariables = true;
        static constexpr bool TypesBeforeArguments = false;

        static constexpr auto VarsTuple = std::make_tuple<decltype(Vars)...>(Vars...);
    };


    /** @brief Deduce a single template type followed by a variadic list of template arguments */
    template<template<typename, auto ...> typename Class, typename Type, auto ...Vars>
        requires (sizeof...(Vars) != 0ul)
    struct TemplateDecomposer<Class<Type, Vars...>>
    {
        static constexpr bool IsTemplate = true;
        static constexpr bool HasTypes = true;
        static constexpr bool HasVariables = true;
        static constexpr bool TypesBeforeArguments = true;

        using TypesTuple = std::tuple<Type>;
        static constexpr auto VarsTuple = std::make_tuple<decltype(Vars)...>(Vars...);
    };

    /** @brief Deduce two template types followed by a variadic list of template arguments */
    template<template<typename, typename, auto ...> typename Class, typename Type1, typename Type2, auto ...Vars>
        requires (sizeof...(Vars) != 0ul)
    struct TemplateDecomposer<Class<Type1, Type2, Vars...>>
    {
        static constexpr bool IsTemplate = true;
        static constexpr bool HasTypes = true;
        static constexpr bool HasVariables = true;
        static constexpr bool TypesBeforeArguments = true;

        using TypesTuple = std::tuple<Type1, Type2>;
        static constexpr auto VarsTuple = std::make_tuple<decltype(Vars)...>(Vars...);
    };

    /** @brief Deduce three template types followed by a variadic list of template arguments */
    template<template<typename, typename, typename, auto ...> typename Class, typename Type1, typename Type2, typename Type3, auto ...Vars>
        requires (sizeof...(Vars) != 0ul)
    struct TemplateDecomposer<Class<Type1, Type2, Type3, Vars...>>
    {
        static constexpr bool IsTemplate = true;
        static constexpr bool HasTypes = true;
        static constexpr bool HasVariables = true;
        static constexpr bool TypesBeforeArguments = true;

        using TypesTuple = std::tuple<Type1, Type2, Type3>;
        static constexpr auto VarsTuple = std::make_tuple<decltype(Vars)...>(Vars...);
    };

    /** @brief Deduce four template types followed by a variadic list of template arguments */
    template<template<typename, typename, typename, typename, auto ...> typename Class, typename Type1, typename Type2, typename Type3, typename Type4, auto ...Vars>
        requires (sizeof...(Vars) != 0ul)
    struct TemplateDecomposer<Class<Type1, Type2, Type3, Type4, Vars...>>
    {
        static constexpr bool IsTemplate = true;
        static constexpr bool HasTypes = true;
        static constexpr bool HasVariables = true;
        static constexpr bool TypesBeforeArguments = true;

        using TypesTuple = std::tuple<Type1, Type2, Type3, Type4>;
        static constexpr auto VarsTuple = std::make_tuple<decltype(Vars)...>(Vars...);
    };


    /** @brief Deduce a single template variable followed by a variadic list of template types */
    template<template<auto, typename ...> typename Class, auto Var, typename ...Types>
        requires (sizeof...(Types) != 0ul)
    struct TemplateDecomposer<Class<Var, Types...>>
    {
        static constexpr bool IsTemplate = true;
        static constexpr bool HasTypes = true;
        static constexpr bool HasVariables = true;
        static constexpr bool TypesBeforeArguments = false;

        using TypesTuple = std::tuple<Types...>;
        static constexpr auto VarsTuple = std::make_tuple<decltype(Var)>(Var);
    };

    /** @brief Deduce two template variables followed by a variadic list of template types */
    template<template<auto, auto, typename ...> typename Class, auto Var1, auto Var2, typename ...Types>
        requires (sizeof...(Types) != 0ul)
    struct TemplateDecomposer<Class<Var1, Var2, Types...>>
    {
        static constexpr bool IsTemplate = true;
        static constexpr bool HasTypes = true;
        static constexpr bool HasVariables = true;
        static constexpr bool TypesBeforeArguments = false;

        using TypesTuple = std::tuple<Types...>;
        static constexpr auto VarsTuple = std::make_tuple<decltype(Var1), decltype(Var2)>(Var1, Var2);
    };

    /** @brief Deduce three template variables followed by a variadic list of template types */
    template<template<auto, auto, auto, typename ...> typename Class, auto Var1, auto Var2, auto Var3, typename ...Types>
        requires (sizeof...(Types) != 0ul)
    struct TemplateDecomposer<Class<Var1, Var2, Var3, Types...>>
    {
        static constexpr bool IsTemplate = true;
        static constexpr bool HasTypes = true;
        static constexpr bool HasVariables = true;
        static constexpr bool TypesBeforeArguments = false;

        using TypesTuple = std::tuple<Types...>;
        static constexpr auto VarsTuple = std::make_tuple<decltype(Var1), decltype(Var2), decltype(Var3)>(Var1, Var2, Var3);
    };

    /** @brief Deduce four template variables followed by a variadic list of template types */
    template<template<auto, auto, auto, auto, typename ...> typename Class, auto Var1, auto Var2, auto Var3, auto Var4, typename ...Types>
        requires (sizeof...(Types) != 0ul)
    struct TemplateDecomposer<Class<Var1, Var2, Var3, Var4, Types...>>
    {
        static constexpr bool IsTemplate = true;
        static constexpr bool HasTypes = true;
        static constexpr bool HasVariables = true;
        static constexpr bool TypesBeforeArguments = false;

        using TypesTuple = std::tuple<Types...>;
        static constexpr auto VarsTuple = std::make_tuple<decltype(Var1), decltype(Var2), decltype(Var3), decltype(Var4)>(Var1, Var2, Var3, Var4);
    };
}