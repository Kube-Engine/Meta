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
class kF::Var
{
public:
    /**
     * @brief Describes how the internal instance is stored
     *
     * Value and ValueTrivial are for copied or moved instances
     * ReferenceVolatile and ReferenceConstant are for assigned references
     */
    enum class StorageType : std::size_t {
        Undefined,
        Value,
        ValueTrivial,
        ReferenceVolatile,
        ReferenceConstant
    };

    /** @brief Small optimisation of Var instance */
    union Cache {
        void *ptr;
        std::byte memory[Meta::Internal::TrivialTypeSizeLimit];
    };

    /** @brief Assigns a type value to a Var */
    template<typename Type>
    static Var Assign(Type &&type) { Var tmp; tmp.assign<Type, false>(std::forward<Type>(type)); return tmp; }

    /** @brief Emplaces a type value to a Var */
    template<typename Type, typename ...Args>
    static Var Emplace(Args &&...args) { Var tmp; tmp.emplace<Type, false>(std::forward<Args>(args)...); return tmp; }

    /** @brief Emplaces a type value to a Var */
    template<typename ...Args>
    static Var Construct(const HashedName name, Args &&...args) { Var tmp; tmp.construct(name, std::forward<Args>(args)...); return tmp; }

    /** @brief Default constructor, instance is empty */
    Var(void) noexcept = default;

    /** @brief Copy constructor, deep copy ! */
    Var(const Var &other) { deepCopy<false>(other); }

    /** @brief Move constructor */
    inline Var(Var &&other) noexcept;

    /** @brief Emplace constructor */
    template<typename Type, std::enable_if_t<!std::is_same_v<std::remove_cvref_t<Type>, Var>>* = nullptr>
    Var(Type &&value) { emplace<std::remove_cvref_t<Type>, false>(std::forward<Type>(value)); }

    /** @brief If not empty, will destruct the internal value */
    ~Var(void) { destruct<false>(); }

    /** @brief Copy assignment, deep copy ! */
    Var &operator=(const Var &other) noexcept { deepCopy<true>(other); return *this; }

    /** @brief Move assignment */
    inline Var &operator=(Var &&other) noexcept;

    /** @brief Checks if the instance is not empty */
    [[nodiscard]] explicit operator bool(void) const noexcept { return _type.operator bool(); }

    /** @brief Assigns a variable internally */
    template<typename Type, bool DestructInstance = true>
    void assign(Type &&other);

    /** @brief Deep copy another variable */
    template<bool CheckIfAssignable = true>
    void deepCopy(const Var &other);

    /** @brief Emplaces a type into the current instance */
    template<typename Type, bool DestructInstance = true, typename ...Args>
    inline void emplace(Args &&...args);

    /** @brief Construct semantic */
    template<typename ...Args>
    void construct(const HashedName name, Args &&...args);

    /** @brief Release internal type */
    template<bool ResetMembers = true>
    inline void destruct(void);

    /** @brief Get internal type */
    [[nodiscard]] Meta::Type type(void) const noexcept { return _type; }

    /** @brief Get internal storage type */
    [[nodiscard]] StorageType storageType(void) const noexcept { return _storageType; }

    /** @brief Get internal constness */
    [[nodiscard]] bool isConstant(void) const noexcept { return _storageType == StorageType::ReferenceConstant; }

    /** @brief Fast check of 'type().isTrivial() && storageType == StorageType::Value' */
    [[nodiscard]] bool isTrivialValue(void) const noexcept { return _storageType == StorageType::ValueTrivial; }

    /** @brief Check if type is void */
    [[nodiscard]] bool isVoid(void) const noexcept { return _type.isVoid(); }

    /** @brief Retreive opaque internal data */
    [[nodiscard]] void *data(void) const noexcept { return isTrivialValue() ? data<true>() : data<false>(); }

    /** @brief Retreive opaque internal data knowing triviality at compile time */
    template<bool IsTrivial>
    [[nodiscard]] void *data(void) const noexcept {
        if constexpr (IsTrivial)
            return const_cast<void *>(reinterpret_cast<const void *>(&_data.memory));
        else
            return const_cast<void *>(_data.ptr);
    }

    /** @brief Retreive internal type as given Type reference (unsafe) */
    template<typename Type>
    [[nodiscard]] Type &as(void) noexcept { return *reinterpret_cast<std::remove_cvref_t<Type> *>(data()); }
    template<typename Type>
    [[nodiscard]] const Type &as(void) const noexcept { return *reinterpret_cast<const std::remove_cvref_t<Type> *>(data()); }

    /** @brief Tries to cast internal to himself or base type (If impossible, will throw in debug or crash in release */
    template<typename Type>
    [[nodiscard]] inline Type &cast(void) noexcept_ndebug;
    template<typename Type>
    [[nodiscard]] inline const Type &cast(void) const noexcept_ndebug;

    /** @brief Tries to cast internal to himself or base type (If impossible, will return nullptr */
    template<typename Type>
    [[nodiscard]] inline Type *tryCast(void) noexcept;
    template<typename Type>
    [[nodiscard]] inline const Type *tryCast(void) const noexcept;

    /** @brief Check if internal is castable to given type */
    template<typename Type>
    [[nodiscard]] bool isCastAble(void) const noexcept { return _type.typeID() == typeid(Type) || type().findBase(Meta::Factory<Type>::Resolve()); }

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
    [[nodiscard]] bool operator==(const Type &rhs) const noexcept { return type() == Meta::Factory<Type>::Resolve() ? as<Type>() == rhs : false; }
    template<typename Type>
    [[nodiscard]] bool operator!=(const Type &rhs) const noexcept { return !operator==(rhs); }

public:
    /**
     * @brief Reserves a non-initialized memory block to allow further construction
     *
     * This function makes access to the instance unsafe
     * You must construct the type manually before trying to use the instance
     */
    void reserve(const Meta::Type type) noexcept_ndebug;

    /**
     * @brief Same as 'reserve' but with compile time knowledge of type triviallity
     */
    template<bool IsTrivial>
    inline void reserve(const Meta::Type type) noexcept_ndebug;

private:
    Meta::Type _type {};
    StorageType _storageType { StorageType::Undefined };
    Cache _data;

    /** @brief Unsafe reference getter used internally */
    [[nodiscard]] void *&dataRef(void) noexcept { return _data.ptr; }
};