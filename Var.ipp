/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Variable
 */

kF::Var::Var(Var &&other) noexcept
{
    if (other.isTrivialValue()) {
        reserve<true>(other.type());
        other.type().moveConstruct(data<true>(), other.data<true>());
    } else {
        std::swap(_type, other._type);
        std::swap(_storageType, other._storageType);
        std::swap(other.dataRef(), dataRef());
    }
}

kF::Var &kF::Var::operator=(Var &&other) noexcept
{
    if (other.isTrivialValue()) {
        destruct<true>();
        reserve<true>(other.type());
        other.type().moveConstruct(data<true>(), other.data<true>());
    } else {
        std::swap(_type, other._type);
        std::swap(_storageType, other._storageType);
        std::swap(other.dataRef(), dataRef());
    }
    return *this;
}

template<typename Type, bool DestructInstance>
void kF::Var::assign(Type &&other)
{
    using DirectType = decltype(other);
    using FlatType = std::remove_cvref_t<Type>;

    constexpr bool IsConst = std::is_const_v<std::remove_reference_t<DirectType>>;

    if constexpr (DestructInstance)
        destruct<false>();
    if constexpr (std::is_same_v<FlatType, Var>) {
        if constexpr (!std::is_lvalue_reference_v<DirectType>)
            *this = std::move(other);
        else if constexpr (IsConst)
            dataRef() = const_cast<void *>(other.data());
        else
            dataRef() = other.data();
        _type = other._type;
    } else if constexpr (!std::is_lvalue_reference_v<DirectType>)
        return emplace<FlatType, false>(std::move(other));
    else {
        if constexpr (IsConst)
            dataRef() = const_cast<void *>(reinterpret_cast<const void *>(&other));
        else
            dataRef() = reinterpret_cast<void *>(&other);
        _type = Meta::Factory<FlatType>::Resolve();
    }
    _storageType = ConstexprTernary(IsConst, StorageType::ReferenceConstant, StorageType::ReferenceVolatile);
}

template<bool CheckIfAssignable>
void kF::Var::deepCopy(const Var &other)
{
    bool assigned = false;

    if constexpr (CheckIfAssignable)
        if (*this && type().typeID() == other.type().typeID()) {
            kFAssert(other.type().isCopyAssignable(),
                throw std::runtime_error("Var::deepCopy: Copy assignment is not supported"));
            other.type().copyAssign(data(), const_cast<void *>(other.data()));
            _type = other.type();
            _storageType = type().isTrivial() ? StorageType::ValueTrivial : StorageType::Value;
            return;
        }
    kFAssert(other._type.isCopyAssignable(),
        throw std::runtime_error("Var::deepCopy: Copy construct is not supported"));
    *this = other.type().copyConstruct(const_cast<void *>(other.data()));
}

template<typename UnarrangedType, bool DestructInstance, typename ...Args>
void kF::Var::emplace(Args &&...args)
{
    using Type = typename Meta::Internal::ArrangeType<UnarrangedType>::Type;

    if constexpr (DestructInstance)
        destruct<false>();
    _type = Meta::Factory<Type>::Resolve();
    if constexpr (std::is_same_v<Type, void>) {
        _storageType = StorageType::Undefined;
    } else if constexpr (Meta::Internal::IsTrivial<Type>) {
        _storageType = StorageType::ValueTrivial;
        new (data<true>()) Type(std::forward<Args>(args)...);
    } else {
        _storageType = StorageType::Value;
        dataRef() = std::malloc(sizeof(Type));
        new (data<false>()) Type(std::forward<Args>(args)...);
    }
}

template<typename ...Args>
void kF::Var::construct(const HashedName name, Args &&...args)
{
    auto type = Meta::Resolver::FindType(name);

    kFAssert(type,
        throw std::runtime_error("Var::construct: Unknown type name"));
    void *ptr;
    if (type.isTrivial()) {
        reserve<true>(type);
        ptr = data<true>();
    } else {
        reserve<false>(type);
        ptr = data<false>();
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

template<bool ResetMembers>
void kF::Var::destruct(void)
{
    if (!_type)
        return;
    switch (_storageType) {
    case StorageType::Value:
        _type.destruct(data<false>());
        std::free(data<false>());
        break;
    case StorageType::ValueTrivial:
        _type.destruct(data<true>());
        break;
    default:
        break;
    }
    if constexpr (ResetMembers) {
        _type = Meta::Type();
        _storageType = StorageType::Undefined;
    }
}

template<typename Type>
Type &kF::Var::cast(void) noexcept_ndebug
{
    kFAssert(isCastAble<Type>(),
        throw std::runtime_error("Var::cast: Invalid cast"));
    return as<Type>();
}

template<typename Type>
const Type &kF::Var::cast(void) const noexcept_ndebug
{
    kFAssert(isCastAble<Type>(),
        throw std::runtime_error("Var::cast: Invalid cast"));
    return as<Type>();
}

template<typename Type>
Type *kF::Var::tryCast(void) noexcept
{
    if (isCastAble<Type>())
        return &as<Type>();
    return nullptr;
}

template<typename Type>
const Type *kF::Var::tryCast(void) const noexcept
{
    if (isCastAble<Type>())
        return &as<Type>();
    return nullptr;
}

template<typename Type>
Type kF::Var::directConvert(void) const
{
    auto ty = Meta::Factory<Type>::Resolve();

    if (type() == ty)
        return as<Type>();
    auto conv = type().findConverter(ty);
    kFAssert(conv,
        throw std::runtime_error("Var::directConvert: Type not convertible"));
    Var res = conv.invoke(*this); // Weird bug : if 'Var' type is not explicit (auto), it won't deduce 'as' member function
    return std::move(res.as<Type>());
}

template<bool IsTrivial>
void kF::Var::reserve(const Meta::Type type) noexcept_ndebug
{
    _type = type;
    if constexpr (IsTrivial)
        _storageType = StorageType::ValueTrivial;
    else {
        _storageType = StorageType::Value;
        dataRef() = std::malloc(_type.typeSize());
        kFAssert(data<false>() != nullptr,
            throw std::runtime_error("Var::reserve: Memory exhausted"));
    }
}