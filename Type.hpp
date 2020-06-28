/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Type
 */

#pragma once

#include <vector>

#include "Base.hpp"

/**
 * @brief Type is used to store meta-data about a type
 */
class kF::Meta::Type
{
public:
    using TypeID = std::type_index;
    using DefaultConstructorFunc = Var(*)(void);
    using CopyConstructorFunc = Var(*)(void *);
    using MoveConstructorFunc = Var(*)(void *);
    using CopyAssignmentFunc = void(*)(void *, void *);
    using MoveAssignmentFunc = void(*)(void *, void *);
    using DestructorFunc = void(*)(void *);
    using UnaryOperatorFunc = Var(*)(const void *);
    using BinaryOperatorFunc = Var(*)(const void *, const Var &);
    using AssignmentOperatorFunc = void(*)(void *, const Var &);
    using ToBoolFunc = bool(*)(const void *);

    struct Descriptor
    {
        /* Type description */
        const TypeID typeID; // Unique type identifier
        HashedName name; // Hashed name of the type
        const std::size_t typeSize; // Size of the type
        const bool isTrivial; // If the type is considered 'trivial' in engine
        const bool isVoid; // If the type is void
        const bool isIntegral; // If the type is an integral number
        const bool isFloating; // If the type is a floating number
        const bool isDouble; // If the type is a floating number
        const bool isPointer; // If the type is a pointer

        /* Type semantics */
        const DefaultConstructorFunc defaultConstructFunc;
        const CopyConstructorFunc copyConstructFunc;
        const MoveConstructorFunc moveConstructFunc;
        const CopyAssignmentFunc copyAssignmentFunc;
        const MoveAssignmentFunc moveAssignmentFunc;
        const DestructorFunc destructFunc;
        const ToBoolFunc toBoolFunc;

        /* Fast unary and binary operators */
        const UnaryOperatorFunc unaryFuncs[static_cast<int>(UnaryOperator::Total)] { nullptr };
        const BinaryOperatorFunc binaryFuncs[static_cast<int>(BinaryOperator::Total)] { nullptr };
        const AssignmentOperatorFunc assignmentFuncs[static_cast<int>(AssignmentOperator::Total)] { nullptr };

        /* Type registerable meta-data */
        std::vector<Constructor> constructors;
        std::vector<Type> bases {};
        std::vector<Converter> converters {};
        std::vector<Function> functions {};
        std::vector<Data> datas {};
        std::vector<Signal> signals {};

        template<typename Type>
        static Descriptor Construct(void) noexcept;
    };

    Type(Descriptor *desc = nullptr) noexcept : _desc(desc) {}
    Type(const Type &other) noexcept = default;
    Type &operator=(const Type &other) = default;
    [[nodiscard]] operator bool(void) const noexcept { return _desc; }
    [[nodiscard]] bool operator==(const Type &other) const noexcept { return _desc == other._desc; }
    [[nodiscard]] bool operator!=(const Type &other) const noexcept { return _desc != other._desc; }

    [[nodiscard]] TypeID typeID(void) const noexcept { return _desc->typeID; }

    [[nodiscard]] HashedName name(void) const noexcept { return _desc->name; }

    [[nodiscard]] std::size_t typeSize(void) const noexcept { return _desc->typeSize; }

    [[nodiscard]] bool isTrivial(void) const noexcept { return _desc->isTrivial; }

    [[nodiscard]] bool isVoid(void) const noexcept { return _desc->isVoid; }

    [[nodiscard]] bool isIntegral(void) const noexcept { return _desc->isIntegral; }

    [[nodiscard]] bool isFloating(void) const noexcept { return _desc->isFloating; }

    [[nodiscard]] bool isDouble(void) const noexcept { return _desc->isDouble; }

    [[nodiscard]] bool isDefaultConstructible(void) const noexcept { return _desc->defaultConstructFunc; }
    [[nodiscard]] Var defaultConstruct(void) const;

    [[nodiscard]] bool isCopyConstructible(void) const noexcept { return _desc->copyConstructFunc; }
    [[nodiscard]] Var copyConstruct(void *data) const;

    [[nodiscard]] bool isMoveConstructible(void) const noexcept { return _desc->moveConstructFunc; }
    [[nodiscard]] Var moveConstruct(void *data) const;

    [[nodiscard]] bool isCopyAssignable(void) const noexcept { return _desc->copyAssignmentFunc; }
    void copyAssign(void *instance, void *data) const { (*_desc->copyAssignmentFunc)(instance, data); }

    [[nodiscard]] bool isMoveAssignable(void) const noexcept { return _desc->moveAssignmentFunc; }
    void moveAssign(void *instance, void *data) const { (*_desc->moveAssignmentFunc)(instance, data); }

    [[nodiscard]] bool isBoolConvertible(void) const noexcept { return _desc->toBoolFunc; }
    [[nodiscard]] bool toBool(const void *instance) const { return (*_desc->toBoolFunc)(instance); }

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