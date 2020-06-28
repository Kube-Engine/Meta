/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Converter
 */

#pragma once

#include "Var.hpp"

/**
 * @brief Converter is used to store meta-data about a type converter
 */
class kF::Meta::Converter
{
public:
    using ConvertSignature = Var(*)(const void *);

    /** @brief Describe a meta converter */
    struct Descriptor
    {
        const Type convertType;
        const ConvertSignature convertFunc;

        /** @brief Construct a Descriptor */
        template<typename Type, ConvertSignature FunctionPtr>
        static Descriptor Construct(void) noexcept;
    };

    /** @brief Construct passing a descriptor instance */
    Converter(const Descriptor *desc = nullptr) noexcept : _desc(desc) {}

    /** @brief Copy constructor */
    Converter(const Converter &other) noexcept = default;

    /** @brief Copy assignment */
    Converter &operator=(const Converter &other) = default;

    /** @brief Fast valid check */
    [[nodiscard]] operator bool(void) const noexcept { return _desc; }

    /** @brief Compare operator */
    [[nodiscard]] bool operator==(const Converter &other) const noexcept { return _desc == other._desc; }

    /** @brief Get target converter type */
    [[nodiscard]] Type convertType(void) const noexcept { return _desc->convertType; }

    /** @brief Invoke the converter */
    [[nodiscard]] Var invoke(const Var &from) const { return (*_desc->convertFunc)(from.data()); }
    [[nodiscard]] Var invoke(const void *from) const { return (*_desc->convertFunc)(from); }

private:
    const Descriptor *_desc = nullptr;
};