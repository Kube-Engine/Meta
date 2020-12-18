/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Tests of TemplateDecomposer
 */

#include <Kube/Meta/TemplateDecomposer.hpp>

using namespace kF::Meta::Internal;

template<typename Type>
struct OneType {};

template<typename Type1, typename Type2>
struct TwoTypes {};

template<typename ...Types>
struct VariadicTypes {};

template<auto Arg>
struct OneArgument {};

template<auto Arg1, auto Arg2>
struct TwoArguments {};

template<auto ...Args>
struct VariadicArguments {};

template<typename Type, auto ...Args>
struct OneTypeVariadicArguments {};

template<typename Type1, typename Type2, auto ...Args>
struct TwoTypesVariadicArguments {};

template<typename Type1, typename Type2, typename Type3, auto ...Args>
struct ThreeTypesVariadicArguments {};

template<typename Type1, typename Type2, typename Type3, typename Type4, auto ...Args>
struct FourTypesVariadicArguments {};

template<auto Arg, typename ...Types>
struct OneArgumentVariadicTypes {};

template<auto Arg1, auto Arg2, typename ...Types>
struct TwoArgumentsVariadicTypes {};

template<auto Arg1, auto Arg2, auto Arg3, typename ...Types>
struct ThreeArgumentsVariadicTypes {};

template<auto Arg1, auto Arg2, auto Arg3, auto Arg4, typename ...Types>
struct FourArgumentsVariadicTypes {};

struct NotTemplate {};

struct NotTemplate2 : public OneType<int> {};


#define ASSERT_TEMPLATE(ClassType, isTemplate, hasTypes, hasVariables, typesBeforeArguments, ...) \
    static_assert(TemplateDecomposer<ClassType __VA_OPT__(<__VA_ARGS__>)>::IsTemplate == isTemplate, "TemplateDecomposer<" #ClassType ">::IsTemplate is not equal to " #isTemplate); \
    static_assert(TemplateDecomposer<ClassType __VA_OPT__(<__VA_ARGS__>)>::HasTypes == hasTypes, "TemplateDecomposer<" #ClassType ">::HasTypes is not equal to " #hasTypes); \
    static_assert(TemplateDecomposer<ClassType __VA_OPT__(<__VA_ARGS__>)>::HasVariables == hasVariables, "TemplateDecomposer<" #ClassType ">::HasVariables is not equal to " #hasVariables); \
    static_assert(TemplateDecomposer<ClassType __VA_OPT__(<__VA_ARGS__>)>::TypesBeforeArguments == typesBeforeArguments, "TemplateDecomposer<" #ClassType ">::TypesBeforeArguments is not equal to " #typesBeforeArguments);

#define ASSERT_TEMPLATE_DECOMPOSER_NOT_TEMPLATE(ClassType) ASSERT_TEMPLATE(ClassType, false, false, false, false)
#define ASSERT_TEMPLATE_DECOMPOSER_TYPES(ClassType, ...) ASSERT_TEMPLATE(ClassType, true, true, false, true, __VA_ARGS__)
#define ASSERT_TEMPLATE_DECOMPOSER_ARGUMENTS(ClassType, ...) ASSERT_TEMPLATE(ClassType, true, false, true, false, __VA_ARGS__)
#define ASSERT_TEMPLATE_DECOMPOSER_TYPES_ARGUMENTS(ClassType, ...) ASSERT_TEMPLATE(ClassType, true, true, true, true, __VA_ARGS__)
#define ASSERT_TEMPLATE_DECOMPOSER_ARGUMENTS_TYPES(ClassType, ...) ASSERT_TEMPLATE(ClassType, true, true, true, false, __VA_ARGS__)

ASSERT_TEMPLATE_DECOMPOSER_NOT_TEMPLATE(int)
ASSERT_TEMPLATE_DECOMPOSER_NOT_TEMPLATE(NotTemplate)
ASSERT_TEMPLATE_DECOMPOSER_NOT_TEMPLATE(NotTemplate2)

ASSERT_TEMPLATE_DECOMPOSER_TYPES(OneType, int)
ASSERT_TEMPLATE_DECOMPOSER_TYPES(TwoTypes, int, float)
ASSERT_TEMPLATE_DECOMPOSER_TYPES(VariadicTypes, int, float, int, float, int)

ASSERT_TEMPLATE_DECOMPOSER_ARGUMENTS(OneArgument, 42)
ASSERT_TEMPLATE_DECOMPOSER_ARGUMENTS(TwoArguments, 42, 42ul)
ASSERT_TEMPLATE_DECOMPOSER_ARGUMENTS(VariadicArguments, 12, 21, 23, 32ul)

ASSERT_TEMPLATE_DECOMPOSER_TYPES_ARGUMENTS(OneTypeVariadicArguments, int, 42)
ASSERT_TEMPLATE_DECOMPOSER_TYPES_ARGUMENTS(TwoTypesVariadicArguments, int, float, 42, 42ll)
ASSERT_TEMPLATE_DECOMPOSER_TYPES_ARGUMENTS(ThreeTypesVariadicArguments, int, float, int, 42, 42, 42ul)
ASSERT_TEMPLATE_DECOMPOSER_TYPES_ARGUMENTS(FourTypesVariadicArguments, int, float, int, float, 42)

ASSERT_TEMPLATE_DECOMPOSER_ARGUMENTS_TYPES(OneArgumentVariadicTypes, 42, int)
ASSERT_TEMPLATE_DECOMPOSER_ARGUMENTS_TYPES(TwoArgumentsVariadicTypes, 42, 42ll, int, float, int)
ASSERT_TEMPLATE_DECOMPOSER_ARGUMENTS_TYPES(ThreeArgumentsVariadicTypes, 0ul, 42ul, 24, float)
ASSERT_TEMPLATE_DECOMPOSER_ARGUMENTS_TYPES(FourArgumentsVariadicTypes, 42ul, 42ll, 42, 24ll, float, int, float, int)
