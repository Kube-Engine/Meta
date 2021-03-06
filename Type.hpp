/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Type
 */

#pragma once

#include <Kube/Core/FlatVector.hpp>
#include <Kube/Core/FlatString.hpp>

#include "Base.hpp"

/**
 * @brief Type is used to store meta-data about a type
 */
class kF::Meta::Type
{
public:
    using TypeID = std::type_index;
    using DefaultConstructorFunc = void(*)(void *);
    using CopyConstructorFunc = void(*)(void *, const void *);
    using MoveConstructorFunc = void(*)(void *, void *);
    using CopyAssignmentFunc = void(*)(void *, const void *);
    using MoveAssignmentFunc = void(*)(void *, void *);
    using DestructorFunc = void(*)(void *);
    using UnaryOperatorFunc = Var(*)(const void *);
    using BinaryOperatorFunc = Var(*)(const void *, const Var &);
    using AssignmentOperatorFunc = void(*)(void *, const Var &);
    using ToBoolFunc = bool(*)(const void *);

    enum Flags : std::uint32_t
    {
        NoFlags             = 0b0,
        IsSmallOptimized    = 0b1,
        IsVoid              = 0b10,
        IsIntegral          = 0b100,
        IsFloating          = 0b1000,
        IsDouble            = 0b10000,
        IsPointer           = 0b100000
    };

    struct alignas_double_cacheline Descriptor
    {
        // --- Cacheline 1 ---
        /* Type description - 32 bytes */
        const TypeID typeID; // Unique type identifier
        const std::uint32_t typeSize; // Size of the type
        const std::uint32_t typeAlignment; // Size of the type
        HashedName name; // Hashed name of the type
        const Flags flags; // Flags that describe the type
        Core::FlatString literal; // Type literal

        /* Type semantics - 56 bytes */
        const DestructorFunc destructFunc;
        const DefaultConstructorFunc defaultConstructFunc;
        const CopyConstructorFunc copyConstructFunc;
        const MoveConstructorFunc moveConstructFunc;
        // --- Cacheline 2 ---
        const CopyAssignmentFunc copyAssignmentFunc;
        const MoveAssignmentFunc moveAssignmentFunc;
        const ToBoolFunc toBoolFunc;

        /* Fast unary and binary operators - 88 bytes */
        const UnaryOperatorFunc unaryFuncs[static_cast<int>(UnaryOperator::Total)] { nullptr };
        const BinaryOperatorFunc binaryFuncs[static_cast<int>(BinaryOperator::Total)] { nullptr };
        const AssignmentOperatorFunc assignmentFuncs[static_cast<int>(AssignmentOperator::Total)] { nullptr };
        char _padding[32]; // Padding to the next cacheline

        // --- Cacheline 4 ---

        /* Type registerable meta-data - 48 bytes */
        Core::FlatVector<Constructor> constructors;
        Core::FlatVector<Type> bases {};
        Core::FlatVector<Converter> converters {};
        Core::FlatVector<Function> functions {};
        Core::FlatVector<Data> datas {};
        Core::FlatVector<Signal> signals {};

        template<typename Type>
        [[nodiscard]] static Descriptor Construct(void) noexcept;
    };

    static_assert_sizeof(Descriptor, Core::CacheLineSize * 4);
    static_assert_alignof_double_cacheline(Descriptor);

    /** @brief Default constructor */
    Type(void) = default;

    /** @brief Construct passing a descriptor instance */
    Type(Descriptor *desc) noexcept : _desc(desc) {}

    /** @brief Copy constructor */
    Type(const Type &other) noexcept = default;

    /** @brief Copy assignment */
    Type &operator=(const Type &other) noexcept = default;

    /** @brief Fast valid check */
    [[nodiscard]] operator bool(void) const noexcept { return _desc; }

    /** @brief Compare operator */
    [[nodiscard]] bool operator==(const Type &other) const noexcept { return _desc == other._desc; }
    [[nodiscard]] bool operator!=(const Type &other) const noexcept { return _desc != other._desc; }

    /** @brief Retreive type's ID */
    [[nodiscard]] TypeID typeID(void) const noexcept { return _desc->typeID; }

    /** @brief Retreive type's name */
    [[nodiscard]] HashedName name(void) const noexcept { return _desc->name; }

    /** @brief Retreive type's name */
    [[nodiscard]] std::string_view literal(void) const noexcept { return _desc->literal.toStdView(); }

    /** @brief Retreive type' size */
    [[nodiscard]] std::size_t typeSize(void) const noexcept { return _desc->typeSize; }

    /** @brief Retreive type' alignment */
    [[nodiscard]] std::size_t typeAlignment(void) const noexcept { return _desc->typeAlignment; }

    /** @brief Check if type is optimized */
    [[nodiscard]] bool isSmallOptimized(void) const noexcept { return _desc->flags & Flags::IsSmallOptimized; }

    /** @brief Check if type is void */
    [[nodiscard]] bool isVoid(void) const noexcept { return _desc->flags & Flags::IsVoid; }

    /** @brief Check if type is an intergral one */
    [[nodiscard]] bool isIntegral(void) const noexcept { return _desc->flags & Flags::IsIntegral; }

    /** @brief Check if type is a floating one */
    [[nodiscard]] bool isFloating(void) const noexcept { return _desc->flags & Flags::IsFloating; }

    /** @brief Check if type is double */
    [[nodiscard]] bool isDouble(void) const noexcept { return _desc->flags & Flags::IsDouble; }

    /** @brief Check if type is pointer */
    [[nodiscard]] bool isPointer(void) const noexcept { return _desc->flags & Flags::IsPointer; }

    /** @brief Check if type is default constructible */
    [[nodiscard]] bool isDefaultConstructible(void) const noexcept { return _desc->defaultConstructFunc; }

    /** @brief Default construct the underlying type */
    void defaultConstruct(void *instance) const { (*_desc->defaultConstructFunc)(instance); }
    [[nodiscard]] Var defaultConstruct(void) const;

    /** @brief Check if type is copy constructible */
    [[nodiscard]] bool isCopyConstructible(void) const noexcept { return _desc->copyConstructFunc; }

    /** @brief Copy construct the underlying type */
    void copyConstruct(void *instance, const void *data) const { (*_desc->copyConstructFunc)(instance, data); }
    [[nodiscard]] Var copyConstruct(const void *data) const;

    /** @brief Check if type is move constructible */
    [[nodiscard]] bool isMoveConstructible(void) const noexcept { return _desc->moveConstructFunc; }

    /** @brief Move construct the underlying type */
    void moveConstruct(void *instance, void *data) const { (*_desc->moveConstructFunc)(instance, data); }
    [[nodiscard]] Var moveConstruct(void *data) const;

    /** @brief Check if type is copy assignable */
    [[nodiscard]] bool isCopyAssignable(void) const noexcept { return _desc->copyAssignmentFunc; }

    /** @brief Copy assign the underlying type */
    void copyAssign(void *instance, void *data) const { (*_desc->copyAssignmentFunc)(instance, data); }

    /** @brief Check if type is move assignable */
    [[nodiscard]] bool isMoveAssignable(void) const noexcept { return _desc->moveAssignmentFunc; }

    /** @brief Move assign the underlying type */
    void moveAssign(void *instance, void *data) const { (*_desc->moveAssignmentFunc)(instance, data); }

    /** @brief Check if type is convertible to boolean */
    [[nodiscard]] bool isBoolConvertible(void) const noexcept { return _desc->toBoolFunc; }

    /** @brief Convert to boolean the underlying type */
    [[nodiscard]] bool toBool(const void *instance) const { return (*_desc->toBoolFunc)(instance); }

    /** @brief Destruct the underlying type */
    void destruct(void *data) const { (*_desc->destructFunc)(data); }

    /** @brief Check the existence of  a meta given Unary / Binary / Assigment operator */
    template<UnaryOperator Operator> [[nodiscard]] bool hasOperator(void) const noexcept { return _desc->unaryFuncs[static_cast<int>(Operator)]; }
    template<BinaryOperator Operator> [[nodiscard]] bool hasOperator(void) const noexcept { return _desc->binaryFuncs[static_cast<int>(Operator)]; }
    template<AssignmentOperator Operator> [[nodiscard]] bool hasOperator(void) const noexcept { return _desc->assignmentFuncs[static_cast<int>(Operator)]; }

    /** @brief Invoke a meta given Unary / Binary / Assigment operator */
    template<UnaryOperator Operator> [[nodiscard]] Var invokeOperator(const void *data) const;
    template<BinaryOperator Operator> [[nodiscard]] Var invokeOperator(const void *data, const Var &rhs) const;
    template<AssignmentOperator Operator> void invokeOperator(void *data, const Var &rhs) const;

    /** @brief Find a registered meta base */
    [[nodiscard]] Type findBase(const Type type) const noexcept;

    /** @brief Find a registered meta converter */
    [[nodiscard]] Converter findConverter(const Type type) const noexcept;

    /** @brief Find a registered meta function */
    [[nodiscard]] Function findFunction(const HashedName name) const noexcept;

    /** @brief Find a registered meta data */
    [[nodiscard]] Data findData(const HashedName name) const noexcept;

    /** @brief Find a registered meta signal by name */
    [[nodiscard]] Signal findSignal(const HashedName name) const noexcept;

    /** @brief Find a registered meta signal by address */
    template<auto SignalPtr>
    [[nodiscard]] Signal findSignal(void) const noexcept;

    /**
     * @brief Find a registered meta constructor with a list of compile time arguments types
     *
     * This function will search for the best matched constructor, allowing implicit conversion
     */
    template<typename ...Args>
    [[nodiscard]] Constructor findConstructor(void) const noexcept;

    /**
     * @brief Find a registered meta constructor with a list runtime meta types
     *
     * This function will search for the best matched constructor, allowing implicit conversion
     */
    [[nodiscard]] Constructor findConstructor(const std::vector<Type> &types) const noexcept;

    /** @brief Clear the registered type meta-data */
    void clear(void);

private:
    Descriptor * _desc = nullptr;
};