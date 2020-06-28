/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Variable
 */

#pragma once

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
    enum class StorageType : std::uint8_t {
        Value,
        Reference
    };

    enum class Constness : bool {
        Volatile,
        Constant
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
    Var(Var &&other) noexcept { swap(other); }

    /** @brief Emplace constructor */
    template<typename Type, std::enable_if_t<!std::is_same_v<std::remove_cvref_t<Type>, Var>>* = nullptr>
    Var(Type &&value) { emplace<std::remove_cvref_t<Type>, false>(std::forward<Type>(value)); }

    /** @brief If not empty, will destruct the internal value */
    ~Var(void) { release(); }

    /** @brief Copy assignment, deep copy ! */
    Var &operator=(const Var &other) noexcept { deepCopy<true>(other); return *this; }

    /** @brief Move assignment */
    Var &operator=(Var &&other) noexcept { swap(other); return *this; }

    /** @brief Checks if the instance is not empty */
    [[nodiscard]] explicit operator bool(void) const noexcept { return _type.operator bool(); }

    /** @brief Release internal type */
    void release(void);

    /** @brief Swap instances */
    void swap(Var &other) noexcept;

    /** @brief Assigns a variable internally */
    template<typename Type, bool ReleaseInstance = true>
    void assign(Type &&other);

    /** @brief Deep copy another variable */
    template<bool CheckIfAssignable = true>
    void deepCopy(const Var &other);

    /** @brief Emplaces a type into the current instance */
    template<typename Type, bool ReleaseInstance = true, typename ...Args>
    void emplace(Args &&...args);

    /** @brief Construct semantic */
    template<typename ...Args>
    void construct(const HashedName name, Args &&...args);

    /** @brief Get internal type */
    [[nodiscard]] Meta::Type type(void) const noexcept { return _type; }

    /** @brief Get internal storage type */
    [[nodiscard]] StorageType storageType(void) const noexcept { return _storageType; }

    /** @brief Get internal constness */
    [[nodiscard]] Constness constness(void) const noexcept { return _constness; }

    /** @brief Fast check of 'type().isTrivial() && storageType == StorageType::Value' */
    [[nodiscard]] bool isTrivialValue(void) const noexcept { return _isTrivialValue; }

    /** @brief Retreive opaque internal data */
    void *data(void) noexcept { return isTrivialValue() ? &_data : _data; }
    const void *data(void) const noexcept { return isTrivialValue() ? &_data : _data; }

    /** @brief Retreive internal type as given Type reference (unsafe) */
    template<typename Type>
    Type &as(void) noexcept { return *reinterpret_cast<Type *>(data()); }
    template<typename Type>
    const Type &as(void) const noexcept { return *reinterpret_cast<const Type *>(data()); }

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
    inline void reserve(const Meta::Type type) noexcept_ndebug;

private:
    void *_data { nullptr };
    Meta::Type _type {};
    StorageType _storageType;
    Constness _constness;
    bool _isTrivialValue;
};