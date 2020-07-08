/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Variable
 */

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
            return swap(other);
        else if constexpr (IsConst)
            _data = const_cast<void *>(other.data());
        else
            _data = other.data();
        _type = other._type;
    } else if constexpr (!std::is_lvalue_reference_v<DirectType>)
        return emplace<FlatType, false>(std::move(other));
    else {
        if constexpr (IsConst)
            _data = const_cast<void *>(reinterpret_cast<const void *>(&other));
        else
            _data = &other;
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
        _data = nullptr;
    } else if constexpr (Meta::Internal::IsTrivial<Type>) {
        _storageType = StorageType::ValueTrivial;
        new (&_data) Type(std::forward<Args>(args)...);
    } else {
        _storageType = StorageType::Value;
        _data = std::malloc(sizeof(Type));
        new (_data) Type(std::forward<Args>(args)...);
    }
}

template<typename ...Args>
void kF::Var::construct(const HashedName name, Args &&...args)
{
    auto type = Meta::Resolver::FindTypeByName(name);

    kFAssert(type,
        throw std::runtime_error("Var::construct: Unknown type name"));
    _storageType = type.isTrivial() ? StorageType::ValueTrivial : StorageType::Value;
    if constexpr (sizeof...(Args) == 0) {
        kFAssert(type.isDefaultConstructible(),
            throw std::runtime_error("Var::construct: Given type is not default constructible"));
        *this = type.defaultConstruct();
    } else if constexpr (sizeof...(Args) == 1 && (type.typeID() == typeid(std::tuple_element_t<0, std::tuple<Args...>>))) {
        if constexpr (std::is_lvalue_reference_v<std::tuple_element_t<0, std::tuple<Args...>>>) {
            kFAssert(type.isCopyConstructible(),
                throw std::runtime_error("Var::construct: Given type is not copy constructible"));
            *this = type.copyConstruct(&args...);
        } else {
            kFAssert(type.isMoveConstructible(),
                throw std::runtime_error("Var::construct: Given type is not move constructible"));
            *this = type.moveConstruct(&args...);
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
        _type.destruct(_data);
        std::free(_data);
        break;
    case StorageType::ValueTrivial:
        _type.destruct(&_data);
        break;
    default:
        break;
    }
    if constexpr (ResetMembers) {
        _data = nullptr;
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

void kF::Var::reserve(const Meta::Type type) noexcept_ndebug
{
    _type = type;
    if (type.isTrivial())
        _storageType = StorageType::ValueTrivial;
    else {
        _storageType = StorageType::Value;
        _data = std::malloc(type.typeSize());
        kFAssert(_data != nullptr,
            throw std::runtime_error("Var::reserve: Memory exhausted"));
    }
}