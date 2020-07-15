/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Type
 */

template<typename UnarrangedType>
kF::Meta::Type::Descriptor kF::Meta::Type::Descriptor::Construct(void) noexcept
{
#define MakeOperatorIf(OpType, Op, Condition, Exact) \
    ConstexprTernary((!std::is_same_v<Type, void>), \
        ConstexprTernary((std::is_integral_v<Type> || Condition || (std::experimental::is_detected_exact_v<Exact, Internal::OpType##Op##Check, Type>)), \
            (&Internal::Make##OpType##Operator<Type, &Internal::OpType##Op<Type>, OpType##Operator::Op>), \
            nullptr \
        ), \
        nullptr \
    )

#define MakeOperatorIfPointerable(OpType, Op, Condition, Exact) \
    ConstexprTernary((std::is_array_v<Type> || std::is_pointer_v<Type>), \
        (&Internal::Make##OpType##Operator<Type, &Internal::OpType##Op##Pointer<Type>, Meta::OpType##Operator::Op>), \
        MakeOperatorIf(OpType, Op, Condition, Exact) \
    )

#define MakeOperatorIfUnary(Op, Condition) MakeOperatorIf(Binary, Op, Condition, Type)
#define MakeOperatorIfBinary(Op, Condition) MakeOperatorIf(Binary, Op, Condition, Type)
#define MakeOperatorIfAssignment(Op, Condition) MakeOperatorIf(Assignment, Op, Condition, Type &)
#define MakeOperatorUnary(Op) MakeOperatorIf(Unary, Op, false, Type)
#define MakeOperatorBinary(Op) MakeOperatorIf(Binary, Op, false, Type)
#define MakeOperatorAssignment(Op) MakeOperatorIf(Assignment, Op, false, Type &)
#define MakeOperatorPointerableUnary(Op) MakeOperatorIfPointerable(Unary, Op, false, Type)
#define MakeOperatorPointerableBinary(Op) MakeOperatorIfPointerable(Binary, Op, false, Type)
#define MakeOperatorPointerableAssignment(Op) MakeOperatorIfPointerable(Assignment, Op, false, Type &)
#define MakeOperatorIfPointerableUnary(Op, Condition) MakeOperatorIfPointerable(Unary, Op, Condition, Type)
#define MakeOperatorIfPointerableBinary(Op, Condition) MakeOperatorIfPointerable(Binary, Op, Condition, Type)
#define MakeOperatorIfPointerableAssignment(Op, Condition) MakeOperatorIfPointerable(Assignment, Op, Condition, Type &)

    using Type = typename Internal::ArrangeType<UnarrangedType>::Type;

    return Descriptor {
        typeID: typeid(Type),
        name: 0,
        typeSize: ConstexprTernary((!std::is_same_v<Type, void>),
            sizeof(Type),
            0u
        ),
        isTrivial: ConstexprTernary((!std::is_same_v<Type, void>),
            Internal::IsTrivial<Type>,
            false
        ),
        isVoid: std::is_same_v<Type, void>,
        isIntegral: std::is_integral_v<Type>,
        isFloating: std::is_floating_point_v<Type>,
        isDouble: std::is_same_v<Type, double>,
        isPointer: std::is_array_v<Type> || std::is_pointer_v<Type>,
        defaultConstructFunc: ConstexprTernary((!std::is_same_v<Type, void> && std::is_default_constructible_v<Type>),
            &Internal::MakeDefaultConstructor<Type>,
            nullptr
        ),
        copyConstructFunc: ConstexprTernary((!std::is_same_v<Type, void> && std::is_copy_constructible_v<Type>),
            &Internal::MakeCopyConstructor<Type>,
            nullptr
        ),
        moveConstructFunc: ConstexprTernary((!std::is_same_v<Type, void> && std::is_move_constructible_v<Type>),
            &Internal::MakeMoveConstructor<Type>,
            nullptr
        ),
        copyAssignmentFunc: ConstexprTernary((!std::is_same_v<Type, void> && std::is_copy_assignable_v<Type>),
            &Internal::MakeCopyAssignment<Type>,
            nullptr
        ),
        moveAssignmentFunc: ConstexprTernary((!std::is_same_v<Type, void> && std::is_move_assignable_v<Type>),
            &Internal::MakeMoveAssignment<Type>,
            nullptr
        ),
        destructFunc: ConstexprTernary(std::is_destructible_v<Type>,
            &Internal::MakeDestructor<Type>,
            nullptr
        ),
        toBoolFunc: ConstexprTernary((std::is_convertible_v<Type, bool> || std::experimental::is_detected_v<Internal::BoolOperatorCheck, Type>),
            &Internal::MakeToBool<Type>,
            nullptr
        ),
        unaryFuncs: {
            MakeOperatorUnary(Minus)
        },
        binaryFuncs: {
            MakeOperatorPointerableBinary(Addition),
            MakeOperatorPointerableBinary(Substraction),
            MakeOperatorBinary(Multiplication),
            MakeOperatorBinary(Division),
            MakeOperatorIfBinary(Modulo, std::is_floating_point_v<Type>)
        },
        assignmentFuncs: {
            MakeOperatorPointerableAssignment(Addition),
            MakeOperatorPointerableAssignment(Substraction),
            MakeOperatorAssignment(Multiplication),
            MakeOperatorAssignment(Division),
            MakeOperatorIfAssignment(Modulo, std::is_floating_point_v<Type>)
        }
    };

#undef MakeOperatorIf
#undef MakeOperatorPointerableIf
#undef MakeOperatorIfUnary
#undef MakeOperatorIfBinary
#undef MakeOperatorIfAssignment
#undef MakeOperatorUnary
#undef MakeOperatorBinary
#undef MakeOperatorAssignment
#undef MakeOperatorPointerableUnary
#undef MakeOperatorPointerableBinary
#undef MakeOperatorPointerableAssignment
#undef MakeOperatorIfPointerableUnary
#undef MakeOperatorIfPointerableBinary
#undef MakeOperatorIfPointerableAssignment
}

template<kF::Meta::UnaryOperator Operator>
kF::Var kF::Meta::Type::invokeOperator(const void *data) const
{
    kFAssert(hasOperator<Operator>(),
        throw std::runtime_error("Meta::Type::invokeOperator: Operator not available"));
    return (*_desc->unaryFuncs[static_cast<int>(Operator)])(data);
}

template<kF::Meta::BinaryOperator Operator>
kF::Var kF::Meta::Type::invokeOperator(const void *data, const Var &rhs) const
{
    kFAssert(hasOperator<Operator>(),
        throw std::runtime_error("Meta::Type::invokeOperator: Operator not available"));
    return (*_desc->binaryFuncs[static_cast<int>(Operator)])(data, rhs);
}

template<kF::Meta::AssignmentOperator Operator>
void kF::Meta::Type::invokeOperator(void *data, const Var &rhs) const
{
    kFAssert(hasOperator<Operator>(),
        throw std::runtime_error("Meta::Type::invokeOperator: Operator not available"));
    return (*_desc->assignmentFuncs[static_cast<int>(Operator)])(data, rhs);
}

template<auto SignalPtr>
kF::Meta::Signal kF::Meta::Type::findSignal(void) const noexcept
{
    const auto signalPtr = Internal::GetFunctionIdentifier<SignalPtr>();

    for (const auto &signal : _desc->signals)
        if (signal.signalPtr() == signalPtr)
            return signal;
    for (Signal res; const auto &base : _desc->bases)
        if (res = base.findSignal<SignalPtr>(); res)
            return res;
    return Signal();
}

template<typename ...Args>
kF::Meta::Constructor kF::Meta::Type::findConstructor(void) const noexcept
{
    using Decomposer = Internal::FunctionDecomposerHelper<void(*)(Args...)>;

    Constructor preferred {};
    auto bestScore = 0u;

    for (const auto ctor : _desc->constructors) {
        auto i = 0u, score = 0u;
        if (sizeof...(Args) != ctor.argsCount())
            continue;
        while (i < sizeof...(Args)) {
            auto type = Decomposer::ArgType(i);
            auto expectedType = ctor.argType(i);
            if (type == expectedType)
                ++score;
            else if (!type.findConverter(expectedType))
                break;
            ++i;
        }
        // We got a perfect match, return it
        if (score == sizeof...(Args))
            return ctor;
        // We got a match but it requires conversion, store it but continue to check if there is a better match
        else if (i == sizeof...(Args) && bestScore < score) {
            bestScore = score;
            preferred = ctor;
        }
    }
    return preferred;
}