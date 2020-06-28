/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Register all basic meta data
 */

#include <string>

#include "Meta.hpp"

using namespace kF;

template <class From, class To, class = To>
struct IsStaticCastable : std::false_type {};

template <class From, class To>
struct IsStaticCastable<From, To, decltype(static_cast<To>(std::declval<From>()))> : std::true_type {};

#define RegisterConverterHelper(From, To) \
    if constexpr (!std::is_same_v<From, To> && IsStaticCastable<From, To>()) \
        kF::Meta::Factory<From>::RegisterConverter<To>(); \

#define RegisterType(Type, Alias) \
    kF::Meta::Factory<Type>::Register(Hash(Alias)); \
    RegisterConverterHelper(Type, bool); \
    RegisterConverterHelper(Type, std::int8_t); \
    RegisterConverterHelper(Type, std::int16_t); \
    RegisterConverterHelper(Type, std::int32_t); \
    RegisterConverterHelper(Type, std::int64_t); \
    RegisterConverterHelper(Type, std::uint8_t); \
    RegisterConverterHelper(Type, std::uint16_t); \
    RegisterConverterHelper(Type, std::uint32_t); \
    RegisterConverterHelper(Type, std::uint64_t); \
    RegisterConverterHelper(Type, float); \
    RegisterConverterHelper(Type, double);

void Meta::RegisterMetadata(void)
{
    RegisterType(bool,              "bool");
    RegisterType(char,              "char");
    RegisterType(std::int16_t,      "short");
    RegisterType(std::int32_t,      "int");
    RegisterType(std::int64_t,      "long");
    RegisterType(std::uint8_t,      "uchar");
    RegisterType(std::uint16_t,     "ushort");
    RegisterType(std::uint32_t,     "uint");
    RegisterType(std::uint64_t,     "ulong");
    RegisterType(float,             "float");
    RegisterType(double,            "double");
    RegisterType(std::string,       "string");
}

#undef RegisterConverterHelper
#undef RegisterType