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
    using ConvertSignature = void(*)(const void *from, void *to);

    /** @brief Describe a meta converter */
    struct KF_ALIGN_QUARTER_CACHELINE Descriptor
    {
        const Type convertType;
        const ConvertSignature convertFunc;

        /** @brief Construct a Descriptor */
        template<typename From, typename To, auto FunctionPtr>
        static Descriptor Construct(void) noexcept;
    };

    static_assert(sizeof(Descriptor) == 16, "Constructor descriptor must take the quarter of a cacheline");
    static_assert(alignof(Descriptor) == 16, "Constructor descriptor must be aligned to the quarter of a cacheline");

    /** @brief Construct passing a descriptor instance */
    Converter(const Descriptor *desc = nullptr) noexcept : _desc(desc) {}

    /** @brief Copy constructor */
    Converter(const Converter &other) noexcept = default;

    /** @brief Copy assignment */
    Converter &operator=(const Converter &other) noexcept = default;

    /** @brief Fast valid check */
    [[nodiscard]] operator bool(void) const noexcept { return _desc; }

    /** @brief Compare operator */
    [[nodiscard]] bool operator==(const Converter &other) const noexcept { return _desc == other._desc; }
    [[nodiscard]] bool operator!=(const Converter &other) const noexcept { return _desc != other._desc; }

    /** @brief Get target converter type */
    [[nodiscard]] Type convertType(void) const noexcept { return _desc->convertType; }

    /** @brief Invoke the converter and return a Var */
    [[nodiscard]] Var invoke(const Var &from) const { Var to; invoke<Var::ShouldDestructInstance::No>(from, to); return to; }

    /** @brief Invoke the converter directly to a Var */
    template<Var::ShouldDestructInstance DestructInstance = Var::ShouldDestructInstance::Yes>
    void invoke(const Var &from, Var &to) const;
    void invoke(const void *from, void *to) const { (*_desc->convertFunc)(from, to); }

private:
    const Descriptor *_desc = nullptr;
};