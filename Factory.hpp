/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Factory
 */

#pragma once

#include "Type.hpp"

namespace kF::Meta
{
    template<typename Type>
    class FactoryBase;
}

/**
 * @brief Factory is used to instantiate meta-data of a given type
 */
template<typename Type>
class kF::Meta::FactoryBase
{
public:
    /** @brief Register templated type's hashed name */
    static void Register(const HashedName name) noexcept_ndebug;

    /** @brief Register a base of templated type */
    template<typename Base>
    static void RegisterBase(void) noexcept_ndebug;

    /** @brief Register a custom constructor (not a default / copy / move constructor) */
    template<typename ...Args>
    static void RegisterConstructor(void) noexcept_ndebug;

    /** @brief Register a converter of templated type */
    template<typename To, auto FunctionPtr = nullptr>
    static void RegisterConverter(void) noexcept_ndebug;

    /** @brief Register a function of templated type */
    template<auto FunctionPtr>
    static void RegisterFunction(const HashedName name) noexcept_ndebug;

    /** @brief Register a data of templated type */
    template<auto GetFunctionPtr, auto SetFunctionPtr = nullptr>
    static void RegisterData(const HashedName name) noexcept_ndebug;

    /**
     * @brief Register a signal of templated type
     *
     * @tparam SignalPtr Signal's function pointer
     * @param name Signal's hashed name
     * @param arguments Signal's hashed arguments names
     */
    template<auto SignalPtr>
    static void RegisterSignal(const HashedName name, std::vector<HashedName> &&arguments) noexcept_ndebug;

    /** @brief Resolve the templated type's meta descriptor */
    [[nodiscard]] inline static Meta::Type Resolve(void) noexcept { return Meta::Type(&Factory<Type>::GetDescriptor()); }

    /** @brief Default construct */
    FactoryBase(void) = default;

    /** @brief Alias of Register function */
    FactoryBase(const HashedName name) { Register(name); }

    /** @brief Alias of Register function */
    FactoryBase &metaRegister(const HashedName name) { Register(name); return *this; }

    /** @brief Alias of RegisterBase function */
    template<typename Base>
    FactoryBase &base(void) noexcept_ndebug { RegisterBase<Base>(); return *this; }

    /** @brief Alias of RegisterConstructor function */
    template<typename ...Args>
    FactoryBase &constructor(void) noexcept_ndebug { RegisterConstructor<Args...>(); return *this; }

    /** @brief Alias of RegisterConverter function */
    template<typename To, auto FunctionPtr = nullptr>
    FactoryBase &converter(void) noexcept_ndebug { RegisterConverter<To, FunctionPtr>(); return *this; }

    /** @brief Alias of RegisterFunction function */
    template<auto FunctionPtr>
    FactoryBase &function(const HashedName name) noexcept_ndebug { RegisterFunction<FunctionPtr>(name); return *this; }

    /** @brief Alias of RegisterData function */
    template<auto GetFunctionPtr, auto SetFunctionPtr = nullptr>
    FactoryBase &data(const HashedName name) noexcept_ndebug { RegisterData<GetFunctionPtr, SetFunctionPtr>(name); return *this; }

    /** @brief Alias of RegisterSignal function */
    template<auto SignalPtr>
    FactoryBase &signal(const HashedName name, std::vector<HashedName> &&arguments) noexcept_ndebug { RegisterSignal<SignalPtr>(name, std::move(arguments)); return *this; }

private:
    /** @brief Static helper used to store the descriptor instance of the templated type */
    static Meta::Type::Descriptor &GetDescriptor(void) {
        static Meta::Type::Descriptor data { Meta::Type::Descriptor::Construct<Type>() };
        return data;
    }
};

template<typename Type>
class kF::Meta::Factory : public FactoryBase<std::remove_cvref_t<Type>> {};