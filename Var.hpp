/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Variable
 */

#pragma once

#include <cstring>

#include "Type.hpp"

/**
 * @brief Var is used as a wrapper for any type
 *
 * It can hold a value, a reference or a rvalue reference.
 * Value that are <= to size of a pointer are cheaply copied.
 */
class alignas(kF::Meta::Internal::VarSmallOptimizationSize > 16ul ? 64ul : 32ul) kF::Var
{
public:
    /**
     * @brief Describes how the internal instance is stored
     *
     * Value and ValueOptimized are for copied or moved instances
     * ReferenceVolatile and ReferenceConstant are for assigned references
     */
    enum class StorageType : std::uint32_t {
        Undefined,
        Value,
        ValueOptimized,
        ReferenceVolatile,
        ReferenceConstant
    };

    /** @brief Small optimisation of Var instance */
    union Cache {
        void *ptr;
        std::byte memory[Meta::Internal::VarSmallOptimizationSize];
    };

    /** @brief A bunch of enum helpers for better template readability */
    enum class UseSmallOptimization     : bool { No = false, Yes = true };
    enum class ShouldResetMembers       : bool { No = false, Yes = true };
    enum class ShouldCheckIfAssignable  : bool { No = false, Yes = true };
    enum class ShouldDestructInstance   : bool { No = false, Yes = true };

    /** @brief Assigns a type value to a Var */
    template<typename Type>
    static Var Assign(Type &&type)
        { Var tmp; tmp.assign<Type, ShouldDestructInstance::No>(std::forward<Type>(type)); return tmp; }

    /** @brief Emplaces a type value to a Var */
    template<typename Type, typename ...Args>
    static Var Emplace(Args &&...args)
        { Var tmp; tmp.emplace<Type, ShouldDestructInstance::No>(std::forward<Args>(args)...); return tmp; }

    /** @brief Emplaces a type value to a Var */
    template<typename ...Args>
    static Var Construct(const HashedName name, Args &&...args)
        { Var tmp; tmp.construct<ShouldDestructInstance::No>(name, std::forward<Args>(args)...); return tmp; }

    /** @brief Default constructor, instance is empty */
    Var(void) noexcept = default;

    /** @brief Copy constructor, deep copy ! */
    Var(const Var &other) { deepCopy<ShouldCheckIfAssignable::No, ShouldDestructInstance::No>(other); }

    /** @brief Move constructor */
    Var(Var &&other) { move<ShouldDestructInstance::No>(other); }

    /** @brief Emplace constructor */
    template<typename Type, std::enable_if_t<!std::is_same_v<std::remove_cvref_t<Type>, Var>>* = nullptr>
    Var(Type &&value)
        noexcept_constructible(std::remove_cvref_t<Type>, Type)
        { emplace<std::remove_cvref_t<Type>, ShouldDestructInstance::No>(std::forward<Type>(value)); }

    /** @brief If not empty, will destruct the internal value */
    ~Var(void) { release<ShouldResetMembers::No>(); }

    /** @brief Copy assignment, deep copy ! */
    Var &operator=(const Var &other) { deepCopy<ShouldCheckIfAssignable::Yes, ShouldDestructInstance::No>(other); return *this; }

    /** @brief Move assignment, either call constructor or move pointers if not small optimized */
    Var &operator=(Var &&other) { move<ShouldDestructInstance::Yes>(other); return *this; }

    /** @brief Destruct the instance and its internal memory */
    template<ShouldResetMembers ResetMembers = ShouldResetMembers::Yes>
    void release(void);

    /** @brief Checks if the instance is not empty */
    [[nodiscard]] explicit operator bool(void) const noexcept { return _type.operator bool(); }

    /** @brief Assigns a variable internally */
    template<typename Type, ShouldDestructInstance DestructInstance = ShouldDestructInstance::Yes>
    void assign(Type &&other);

    /** @brief Deep copy another variable */
    template<ShouldCheckIfAssignable CheckIfAssignable = ShouldCheckIfAssignable::Yes, ShouldDestructInstance DestructInstance = ShouldDestructInstance::Yes>
    void deepCopy(const Var &other);

    /** @brief Emplaces a type into the current instance */
    template<typename Type, ShouldDestructInstance DestructInstance = ShouldDestructInstance::Yes, typename ...Args>
    void emplace(Args &&...args)
        noexcept(DestructInstance == ShouldDestructInstance::No && nothrow_constructible(Type, Args...));

    /** @brief Construct semantic */
    template<ShouldDestructInstance DestructInstance = ShouldDestructInstance::Yes, typename ...Args>
    void construct(const HashedName name, Args &&...args);

    /** @brief Release internal type */
    template<ShouldResetMembers ResetMembers = ShouldResetMembers::Yes>
    void destruct(void);

    /** @brief Get internal type */
    [[nodiscard]] Meta::Type type(void) const noexcept { return _type; }

    /** @brief Get internal storage type */
    [[nodiscard]] StorageType storageType(void) const noexcept { return _storageType; }

    /** @brief Get internal constness */
    [[nodiscard]] bool isConstant(void) const noexcept { return _storageType == StorageType::ReferenceConstant; }

    /** @brief Fast check of 'type().isTrivial() && storageType == StorageType::Value' */
    [[nodiscard]] bool isSmallOptimizedValue(void) const noexcept { return _storageType == StorageType::ValueOptimized; }

    /** @brief Check if type is void */
    [[nodiscard]] bool isVoid(void) const noexcept { return _type.isVoid(); }

    /** @brief Retreive opaque internal data */
    [[nodiscard]] void *data(void) const noexcept
        { return isSmallOptimizedValue() ? data<UseSmallOptimization::Yes>() : data<UseSmallOptimization::No>(); }

    /** @brief Retreive opaque internal data knowing small optimization state at compile time */
    template<UseSmallOptimization IsSmallOptimized>
    [[nodiscard]] void *data(void) const noexcept;

    /** @brief Retreive internal type as given Type reference (unsafe) */
    template<typename Type>
    [[nodiscard]] Type &as(void) noexcept { return *reinterpret_cast<std::remove_cvref_t<Type> *>(data()); }
    template<typename Type>
    [[nodiscard]] const Type &as(void) const noexcept { return *reinterpret_cast<const std::remove_cvref_t<Type> *>(data()); }

    /** @brief Tries to cast internal to himself or base type (If impossible, will throw in debug or crash in release */
    template<typename Type>
    [[nodiscard]] Type &cast(void) noexcept_ndebug;
    template<typename Type>
    [[nodiscard]] const Type &cast(void) const noexcept_ndebug;

    /** @brief Tries to cast internal to himself or base type (If impossible, will return nullptr */
    template<typename Type>
    [[nodiscard]] Type *tryCast(void) noexcept;
    template<typename Type>
    [[nodiscard]] const Type *tryCast(void) const noexcept;

    /** @brief Check if internal is castable to given type */
    template<typename Type>
    [[nodiscard]] bool isCastAble(void) const noexcept
        { return _type.typeID() == typeid(Type) || type().findBase(Meta::Factory<Type>::Resolve()); }

    /** @brief Tries to convert internal instance to given meta type name */
    [[nodiscard]] Var convert(const Meta::Type type) const;

    /** @brief Tries to convert internal instance to given type */
    template<typename Type>
    [[nodiscard]] Var convert(void) const { return convert(Meta::Factory<Type>::Resolve()); }

    /** @brief Tries to convert internal directly to given type (If impossible, will throw in debug or crash in release) */
    template<typename Type>
    [[nodiscard]] Type directConvert(void) const;

    /** @brief Tries to convert internal to boolean */
    [[nodiscard]] bool toBool(void) const;

    /** @brief Various opaque operators helpers */
    [[nodiscard]] Var operator+(const Var &rhs) const;
    [[nodiscard]] Var operator-(const Var &rhs) const;
    [[nodiscard]] Var operator*(const Var &rhs) const;
    [[nodiscard]] Var operator/(const Var &rhs) const;
    [[nodiscard]] Var operator%(const Var &rhs) const;
    Var &operator+=(const Var &rhs);
    Var &operator-=(const Var &rhs);
    Var &operator*=(const Var &rhs);
    Var &operator/=(const Var &rhs);
    Var &operator%=(const Var &rhs);

    /** @brief Various non-opaque operators helpers */
    template<typename Type>
    [[nodiscard]] bool operator==(const Type &rhs) const noexcept
        { return type() == Meta::Factory<Type>::Resolve() ? as<Type>() == rhs : false; }
    template<typename Type>
    [[nodiscard]] bool operator!=(const Type &rhs) const noexcept { return !operator==(rhs); }

public:
    /**
     * @brief Reserves a non-initialized memory block to allow further construction
     *
     * This function makes access to the instance unsafe
     * You must construct the type manually before trying to use the instance
     */
    template<ShouldDestructInstance DestructInstance = ShouldDestructInstance::Yes>
    void reserve(const Meta::Type type) noexcept_ndebug;

    /** @brief Same as 'reserve' but with compile time knowledge of type triviallity */
    template<UseSmallOptimization IsSmallOptimized, ShouldDestructInstance DestructInstance>
    void reserve(const Meta::Type type) noexcept_ndebug;

private:
    Meta::Type _type {};
    StorageType _storageType { StorageType::Undefined };
    std::uint32_t _capacity { 0 };
    Cache _data;

    /** @brief Unsafe reference getter used internally */
    [[nodiscard]] void *&dataRef(void) noexcept { return _data.ptr; }

    /** @brief Move helper */
    template<ShouldDestructInstance DestructInstance = ShouldDestructInstance::Yes>
    void move(Var &other);

    /** @brief Allocate a given capacity on the heap */
    void alloc(const std::uint32_t capacity) noexcept_ndebug;

    /** @brief Release the current allocation if there is any */
    template<ShouldDestructInstance DestructInstance>
    void releaseAlloc(void) noexcept;
};

static_assert(sizeof(kF::Var) - kF::Meta::Internal::VarSmallOptimizationSize == 16ul, "Var data must take only 16 bytes");