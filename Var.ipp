/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Variable
 */

template<kF::Var::ShouldResetMembers ResetMembers>
inline void kF::Var::release(void)
{
    destruct<ResetMembers>();
    if (_capacity) {
        Core::Utils::AlignedFree(dataRef());
        if constexpr (ResetMembers == ShouldResetMembers::Yes)
            _capacity = 0u;
    }
}

template<typename Type, kF::Var::ShouldDestructInstance DestructInstance>
inline void kF::Var::assign(Type &&other)
{
    using DirectType = decltype(other);
    using FlatType = std::remove_cvref_t<Type>;

    constexpr bool IsConst = std::is_const_v<std::remove_reference_t<DirectType>>;

    if constexpr (DestructInstance == ShouldDestructInstance::Yes)
        destruct<ShouldResetMembers::No>();
    if constexpr (std::is_same_v<FlatType, Var>) {
        if constexpr (!std::is_lvalue_reference_v<DirectType>) {
            move(other);
            return;
        } else {
            releaseAlloc<DestructInstance>();
            if constexpr (IsConst)
                dataRef() = const_cast<void *>(other.data());
            else
                dataRef() = other.data();
        }
        _type = other.type();
    } else if constexpr (!std::is_lvalue_reference_v<DirectType>)
        return emplace<FlatType, ShouldDestructInstance::No>(std::move(other));
    else {
        releaseAlloc<DestructInstance>();
        if constexpr (IsConst)
            dataRef() = const_cast<void *>(reinterpret_cast<const void *>(&other));
        else
            dataRef() = reinterpret_cast<void *>(&other);
        _type = Meta::Factory<FlatType>::Resolve();
    }
    _storageType = ConstexprTernary(IsConst, StorageType::ReferenceConstant, StorageType::ReferenceVolatile);
}

template<kF::Var::ShouldCheckIfAssignable CheckIfAssignable, kF::Var::ShouldDestructInstance DestructInstance>
inline void kF::Var::deepCopy(const Var &other)
{
    if (!other) [[unlikely]] {
        destruct<ShouldResetMembers::Yes>();
        return;
    }
    const auto otherType = other.type();
    kFAssert(otherType.isCopyAssignable(),
        throw std::runtime_error("Var::deepCopy: Copy construct is not supported on type"));
    if constexpr (CheckIfAssignable == ShouldCheckIfAssignable::Yes) {
        if (*this && type().typeID() == otherType.typeID()) {
            if (otherType.isSmallOptimized()) {
                otherType.copyAssign(data<UseSmallOptimization::Yes>(), const_cast<void *>(other.data()));
                _storageType = StorageType::ValueOptimized;
            } else {
                otherType.copyAssign(data<UseSmallOptimization::No>(), const_cast<void *>(other.data()));
                _storageType = StorageType::Value;
            }
            return;
        }
    }
    if (otherType.isSmallOptimized()) {
        reserve<UseSmallOptimization::Yes, DestructInstance>(otherType);
        other.type().copyConstruct(data<UseSmallOptimization::Yes>(), const_cast<void *>(other.data()));
    } else {
        reserve<UseSmallOptimization::No, DestructInstance>(otherType);
        otherType.copyConstruct(data<UseSmallOptimization::No>(), const_cast<void *>(other.data()));
    }
}

template<typename UnarrangedType, kF::Var::ShouldDestructInstance DestructInstance, typename ...Args>
inline void kF::Var::emplace(Args &&...args)
    noexcept(DestructInstance == kF::Var::ShouldDestructInstance::No && nothrow_constructible(UnarrangedType, Args...))
{
    using Type = typename Meta::Internal::ArrangeType<UnarrangedType>::Type;

    if constexpr (DestructInstance == ShouldDestructInstance::Yes)
        destruct<ShouldResetMembers::No>();
    _type = Meta::Factory<Type>::Resolve();
    if constexpr (std::is_same_v<Type, void>) {
        _storageType = StorageType::Undefined;
        releaseAlloc<DestructInstance>();
    } else if constexpr (Meta::Internal::IsVarSmallOptimized<Type>) {
        _storageType = StorageType::ValueOptimized;
        releaseAlloc<DestructInstance>();
        new (data<UseSmallOptimization::Yes>()) Type(std::forward<Args>(args)...);
    } else {
        _storageType = StorageType::Value;
        alloc(sizeof(Type));
        new (data<UseSmallOptimization::No>()) Type(std::forward<Args>(args)...);
    }
}

template<kF::Var::ShouldDestructInstance DestructInstance, typename ...Args>
inline void kF::Var::construct(const HashedName name, Args &&...args)
{
    auto type = Meta::Resolver::FindType(name);

    kFAssert(type,
        throw std::runtime_error("Var::construct: Unknown type name"));
    void *ptr;
    if (type.isSmallOptimized()) {
        reserve<UseSmallOptimization::Yes, DestructInstance>(type);
        ptr = data<UseSmallOptimization::Yes>();
    } else {
        reserve<UseSmallOptimization::No, DestructInstance>(type);
        ptr = data<UseSmallOptimization::No>();
    }
    if constexpr (sizeof...(Args) == 0) {
        kFAssert(_type.isDefaultConstructible(),
            throw std::runtime_error("Var::construct: Given type is not default constructible"));
        _type.defaultConstruct(ptr);
    } else if (sizeof...(Args) == 1 && (_type.typeID() == typeid(std::tuple_element_t<0, std::tuple<Args...>>))) {
        if constexpr (std::is_lvalue_reference_v<std::tuple_element_t<0, std::tuple<Args...>>>) {
            kFAssert(_type.isCopyConstructible(),
                throw std::runtime_error("Var::construct: Given type is not copy constructible"));
            _type.copyConstruct(ptr, &args...);
        } else {
            kFAssert(_type.isMoveConstructible(),
                throw std::runtime_error("Var::construct: Given type is not move constructible"));
            _type.moveConstruct(ptr, &args...);
        }
    } else // Constructor detector
        throw std::runtime_error("Custom constructors not handled now");
}

template<kF::Var::ShouldResetMembers ResetMembers>
inline void kF::Var::destruct(void)
{
    if (!_type) [[unlikely]]
        return;
    switch (_storageType) {
    case StorageType::Value:
        _type.destruct(data<UseSmallOptimization::No>());
        break;
    case StorageType::ValueOptimized:
        _type.destruct(data<UseSmallOptimization::Yes>());
        break;
    default:
        break;
    }
    if constexpr (ResetMembers == ShouldResetMembers::Yes) {
        _type = Meta::Type();
        _storageType = StorageType::Undefined;
    }
}

template<kF::Var::UseSmallOptimization IsSmallOptimized>
inline void *kF::Var::data(void) const noexcept
{
    if constexpr (IsSmallOptimized == UseSmallOptimization::Yes)
        return const_cast<void *>(reinterpret_cast<const void *>(&_data.memory));
    else
        return const_cast<void *>(_data.ptr);
}

template<typename Type>
inline Type &kF::Var::cast(void) noexcept_ndebug
{
    kFAssert(isCastAble<Type>(),
        throw std::runtime_error("Var::cast: Invalid cast"));
    return as<Type>();
}

template<typename Type>
inline const Type &kF::Var::cast(void) const noexcept_ndebug
{
    kFAssert(isCastAble<Type>(),
        throw std::runtime_error("Var::cast: Invalid cast"));
    return as<Type>();
}

template<typename Type>
inline Type *kF::Var::tryCast(void) noexcept
{
    if (isCastAble<Type>())
        return &as<Type>();
    return nullptr;
}

template<typename Type>
inline const Type *kF::Var::tryCast(void) const noexcept
{
    if (isCastAble<Type>())
        return &as<Type>();
    return nullptr;
}

inline kF::Var kF::Var::convertOpaque(const Meta::Type type) const
{
    if (auto conv = _type.findConverter(type); conv)
        return conv.invoke(*this);
    else
        return Var();
}

inline bool kF::Var::convert(const Meta::Type type)
{
    auto conv = _type.findConverter(type);

    if (!conv) [[unlikely]]
        return false;
    Var toMove = conv.invoke(*this);
    move(toMove);
    return true;
}

template<typename To>
inline To kF::Var::convertExplicit(void) const
{
    const auto ty = Meta::Factory<To>::Resolve();
    auto conv = type().findConverter(ty);

    kFAssert(conv,
        throw std::runtime_error("Var::convertExplicit: Type not convertible"));
    To to;
    conv.invoke(data(), &to);
    return to;
}

inline bool kF::Var::toBool(void) const
{
    kFAssert(type().isBoolConvertible(),
        throw std::logic_error("Var::toBool: Boolean operator doesn't exists"));
    return type().toBool(data());
}

inline kF::Var kF::Var::operator+(const Var &rhs) const
{
    kFAssert(type().hasOperator<Meta::BinaryOperator::Addition>(),
        throw std::logic_error("Var::operator+: Addition operator doesn't exists"));
    return type().invokeOperator<Meta::BinaryOperator::Addition>(data(), rhs);
}

inline kF::Var kF::Var::operator-(const Var &rhs) const
{
    kFAssert(type().hasOperator<Meta::BinaryOperator::Substraction>(),
        throw std::logic_error("Var::operator-: Substraction operator doesn't exists"));
    return type().invokeOperator<Meta::BinaryOperator::Substraction>(data(), rhs);
}

inline kF::Var kF::Var::operator*(const Var &rhs) const
{
    kFAssert(type().hasOperator<Meta::BinaryOperator::Multiplication>(),
        throw std::logic_error("Var::operator*: Multiplication operator doesn't exists"));
    return type().invokeOperator<Meta::BinaryOperator::Multiplication>(data(), rhs);
}

inline kF::Var kF::Var::operator/(const Var &rhs) const
{
    kFAssert(type().hasOperator<Meta::BinaryOperator::Division>(),
        throw std::logic_error("Var::operator/: Division operator doesn't exists"));
    return type().invokeOperator<Meta::BinaryOperator::Division>(data(), rhs);
}

inline kF::Var kF::Var::operator%(const Var &rhs) const
{
    kFAssert(type().hasOperator<Meta::BinaryOperator::Modulo>(),
        throw std::logic_error("Var::operator%: Modulo operator doesn't exists"));
    return type().invokeOperator<Meta::BinaryOperator::Modulo>(data(), rhs);
}

inline kF::Var &kF::Var::operator+=(const Var &rhs)
{
    kFAssert(type().hasOperator<Meta::AssignmentOperator::Addition>(),
        throw std::logic_error("Var::operator+=: Addition operator doesn't exists"));
    type().invokeOperator<Meta::AssignmentOperator::Addition>(data(), rhs);
    return *this;
}

inline kF::Var &kF::Var::operator-=(const Var &rhs)
{
    kFAssert(type().hasOperator<Meta::AssignmentOperator::Substraction>(),
        throw std::logic_error("Var::operator-=: Substraction operator doesn't exists"));
    type().invokeOperator<Meta::AssignmentOperator::Substraction>(data(), rhs);
    return *this;
}

inline kF::Var &kF::Var::operator*=(const Var &rhs)
{
    kFAssert(type().hasOperator<Meta::AssignmentOperator::Multiplication>(),
        throw std::logic_error("Var::operator*=: Multiplication operator doesn't exists"));
    type().invokeOperator<Meta::AssignmentOperator::Multiplication>(data(), rhs);
    return *this;
}

inline kF::Var &kF::Var::operator/=(const Var &rhs)
{
    kFAssert(type().hasOperator<Meta::AssignmentOperator::Division>(),
        throw std::logic_error("Var::operator/=: Division operator doesn't exists"));
    type().invokeOperator<Meta::AssignmentOperator::Division>(data(), rhs);
    return *this;
}

inline kF::Var &kF::Var::operator%=(const Var &rhs)
{
    kFAssert(type().hasOperator<Meta::AssignmentOperator::Modulo>(),
        throw std::logic_error("Var::operator%=: Modulo operator doesn't exists"));
    type().invokeOperator<Meta::AssignmentOperator::Modulo>(data(), rhs);
    return *this;
}

template<kF::Var::ShouldDestructInstance DestructInstance>
inline void kF::Var::move(Var &other)
{
    if (other.isSmallOptimizedValue()) {
        reserve<UseSmallOptimization::Yes, DestructInstance>(other.type());
        other.type().moveConstruct(data<UseSmallOptimization::Yes>(), other.data<UseSmallOptimization::Yes>());
    } else {
        if constexpr (DestructInstance == ShouldDestructInstance::Yes) {
            destruct<ShouldResetMembers::Yes>();
            releaseAlloc<DestructInstance>();
        }
        _type = other._type;
        _storageType = other._storageType;
        _capacity = other._capacity;
        dataRef() = other.dataRef();
        other._type = Meta::Type();
        other._storageType = StorageType::Undefined;
        other._capacity = 0;
    }
}

template<kF::Var::ShouldDestructInstance DestructInstance>
inline void kF::Var::reserve(const Meta::Type type) noexcept_ndebug
{
    if (type.isSmallOptimized())
        reserve<UseSmallOptimization::Yes, DestructInstance>(type);
    else
        reserve<UseSmallOptimization::No, DestructInstance>(type);
}

template<kF::Var::UseSmallOptimization IsSmallOptimized, kF::Var::ShouldDestructInstance DestructInstance>
inline void kF::Var::reserve(const Meta::Type type) noexcept_ndebug
{
    if constexpr (DestructInstance == ShouldDestructInstance::Yes)
        destruct<ShouldResetMembers::No>();
    _type = type;
    if constexpr (IsSmallOptimized == UseSmallOptimization::Yes) {
        releaseAlloc<DestructInstance>();
        _storageType = StorageType::ValueOptimized;
    } else {
        _storageType = StorageType::Value;
        alloc(type.typeSize());
    }
}

inline void kF::Var::alloc(const std::uint32_t capacity) noexcept_ndebug
{
    if (_capacity < capacity) [[likely]] {
        if (_capacity) [[unlikely]]
            Core::Utils::AlignedFree(data<UseSmallOptimization::No>());
        dataRef() = Core::Utils::AlignedAlloc(type().typeSize(), type().typeAlignment());
        _capacity = capacity;
        kFAssertFallback(data<UseSmallOptimization::No>() != nullptr,
            _capacity = 0,
            throw std::runtime_error("Var::reserve: Memory exhausted")
        );
    }
}

template<kF::Var::ShouldDestructInstance DestructInstance>
inline void kF::Var::releaseAlloc(void) noexcept
{
    if constexpr (DestructInstance == ShouldDestructInstance::Yes) {
        if (_capacity)
            Core::Utils::AlignedFree(data<UseSmallOptimization::No>());
        _capacity = 0;
    }
}