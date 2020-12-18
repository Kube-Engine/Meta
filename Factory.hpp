/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Factory
 */

#pragma once

#include "Type.hpp"

/**
 * @brief Factory is used to instantiate meta-data of a given type
 */
template<typename RegisteredType>
class kF::Meta::FactoryBase
{
public:
    /** @brief Resolve the templated type's meta descriptor */
    [[nodiscard]] static Type Resolve(void) noexcept
        { return Type(&_Descriptor); }


    /** @brief Register templated type's hashed name */
    static void Register(const HashedName name) noexcept_ndebug
        { Register(name, name); }

    /** @brief Register templated type's hashed name with a specialization */
    static void Register(const HashedName name, const HashedName specialization) noexcept_ndebug;

    /** @brief Register a base of templated type */
    template<typename Base>
    static void RegisterBase(void) noexcept_ndebug;

    /** @brief Register a custom constructor (not a default / copy / move constructor) */
    template<typename ...Args>
    static Constructor RegisterConstructor(void) noexcept_ndebug;

    /** @brief Register a converter of templated type */
    template<typename To, auto FunctionPtr = nullptr>
    static Converter RegisterConverter(void) noexcept_ndebug;

    /** @brief Register a function of templated type */
    template<auto FunctionPtr>
    static Function RegisterFunction(const HashedName name) noexcept_ndebug;

    /** @brief Register a data of templated type with a single setter */
    template<auto GetFunctionPtr, auto SetFunctionPtr, auto SignalPtr = nullptr>
    static Data RegisterData(const HashedName name, const HashedName signalName = 0u) noexcept_ndebug;

    /** @brief Register a data of templated type with a copy and a move setter */
    template<auto GetFunctionPtr, auto SetCopyFunctionPtr, auto SetMoveFunctionPtr, auto SignalPtr>
    static Data RegisterData(const HashedName name, const HashedName signalName = 0u) noexcept_ndebug;

    /**
     * @brief Register a signal of templated type
     *
     * @tparam SignalPtr Signal's function pointer
     * @param name Signal's hashed name
     * @param arguments Signal's hashed arguments names
     */
    template<auto SignalPtr>
    static Signal RegisterSignal(const HashedName name) noexcept_ndebug;


    /** @brief Default construct */
    FactoryBase(void) noexcept = default;

    /** @brief Alias of Register function */
    FactoryBase(const HashedName name)
        { Register(name); }

    /** @brief Alias of Register specialization function */
    FactoryBase(const HashedName name, const HashedName specialization)
        { Register(name, specialization); }

    /** @brief Destructor */
    ~FactoryBase(void) noexcept = default;


    /** @brief Alias of Register function */
    FactoryBase &metaRegister(const HashedName name) noexcept_ndebug
        { Register(name); return *this; }

    /** @brief Alias of Register specialization function */
    FactoryBase &metaRegister(const HashedName name, const HashedName specialization) noexcept_ndebug
        { Register(name, specialization); return *this; }


    /** @brief Alias of RegisterBase function */
    template<typename Base>
    FactoryBase &base(void) noexcept_ndebug
        { RegisterBase<Base>(); return *this; }

    /** @brief Alias of RegisterConstructor function */
    template<typename ...Args>
    FactoryBase &constructor(void) noexcept_ndebug
        { RegisterConstructor<Args...>(); return *this; }

    /** @brief Alias of RegisterConverter function */
    template<typename To, auto FunctionPtr = nullptr>
    FactoryBase &converter(void) noexcept_ndebug
        { RegisterConverter<To, FunctionPtr>(); return *this; }

    /** @brief Alias of RegisterFunction function */
    template<auto FunctionPtr>
    FactoryBase &function(const HashedName name) noexcept_ndebug
        { RegisterFunction<FunctionPtr>(name); return *this; }

    /** @brief Alias of RegisterData function */
    template<auto GetFunctionPtr, auto SetFunctionPtr, auto SignalPtr = nullptr>
    FactoryBase &data(const HashedName name, const HashedName signalName = 0u) noexcept_ndebug
        { RegisterData<GetFunctionPtr, SetFunctionPtr, SignalPtr>(name, signalName); return *this; }

    /** @brief Alias of RegisterData function */
    template<auto GetFunctionPtr, auto SetCopyFunctionPtr, auto SetMoveFunctionPtr, auto SignalPtr>
    FactoryBase &data(const HashedName name, const HashedName signalName = 0u) noexcept_ndebug
        { RegisterData<GetFunctionPtr, SetCopyFunctionPtr, SetMoveFunctionPtr, SignalPtr>(name, signalName); return *this; }

    /** @brief Alias of RegisterSignal function */
    template<auto SignalPtr>
    FactoryBase &signal(const HashedName name) noexcept_ndebug
        { RegisterSignal<SignalPtr>(name); return *this; }

private:
    /** @brief Static helper used to store the descriptor instance of the templated type */
    static inline Type::Descriptor _Descriptor { Type::Descriptor::Construct<RegisteredType>() };
};