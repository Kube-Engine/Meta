/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Variable
 */

#include "Meta.hpp"

using namespace kF;

Var Var::convert(const Meta::Type type) const
{
    if (auto conv = _type.findConverter(type); conv)
        return conv.invoke(*this);
    return Var();
}

bool Var::toBool(void) const
{
    kFAssert(type().isBoolConvertible(),
        throw std::logic_error("Var::toBool: Boolean operator doesn't exists"));
    return type().toBool(data());
}

Var Var::operator+(const Var &rhs) const
{
    kFAssert(type().hasOperator<Meta::BinaryOperator::Addition>(),
        throw std::logic_error("Var::operator+: Addition operator doesn't exists"));
    return type().invokeOperator<Meta::BinaryOperator::Addition>(data(), rhs);
}

Var Var::operator-(const Var &rhs) const
{
    kFAssert(type().hasOperator<Meta::BinaryOperator::Substraction>(),
        throw std::logic_error("Var::operator-: Substraction operator doesn't exists"));
    return type().invokeOperator<Meta::BinaryOperator::Substraction>(data(), rhs);
}

Var Var::operator*(const Var &rhs) const
{
    kFAssert(type().hasOperator<Meta::BinaryOperator::Multiplication>(),
        throw std::logic_error("Var::operator*: Multiplication operator doesn't exists"));
    return type().invokeOperator<Meta::BinaryOperator::Multiplication>(data(), rhs);
}

Var Var::operator/(const Var &rhs) const
{
    kFAssert(type().hasOperator<Meta::BinaryOperator::Division>(),
        throw std::logic_error("Var::operator/: Division operator doesn't exists"));
    return type().invokeOperator<Meta::BinaryOperator::Division>(data(), rhs);
}

Var Var::operator%(const Var &rhs) const
{
    kFAssert(type().hasOperator<Meta::BinaryOperator::Modulo>(),
        throw std::logic_error("Var::operator%: Modulo operator doesn't exists"));
    return type().invokeOperator<Meta::BinaryOperator::Modulo>(data(), rhs);
}

Var &Var::operator+=(const Var &rhs)
{
    kFAssert(type().hasOperator<Meta::AssignmentOperator::Addition>(),
        throw std::logic_error("Var::operator+=: Addition operator doesn't exists"));
    type().invokeOperator<Meta::AssignmentOperator::Addition>(data(), rhs);
    return *this;
}

Var &Var::operator-=(const Var &rhs)
{
    kFAssert(type().hasOperator<Meta::AssignmentOperator::Substraction>(),
        throw std::logic_error("Var::operator-=: Substraction operator doesn't exists"));
    type().invokeOperator<Meta::AssignmentOperator::Substraction>(data(), rhs);
    return *this;
}

Var &Var::operator*=(const Var &rhs)
{
    kFAssert(type().hasOperator<Meta::AssignmentOperator::Multiplication>(),
        throw std::logic_error("Var::operator*=: Multiplication operator doesn't exists"));
    type().invokeOperator<Meta::AssignmentOperator::Multiplication>(data(), rhs);
    return *this;
}

Var &Var::operator/=(const Var &rhs)
{
    kFAssert(type().hasOperator<Meta::AssignmentOperator::Division>(),
        throw std::logic_error("Var::operator/=: Division operator doesn't exists"));
    type().invokeOperator<Meta::AssignmentOperator::Division>(data(), rhs);
    return *this;
}

Var &Var::operator%=(const Var &rhs)
{
    kFAssert(type().hasOperator<Meta::AssignmentOperator::Modulo>(),
        throw std::logic_error("Var::operator%=: Modulo operator doesn't exists"));
    type().invokeOperator<Meta::AssignmentOperator::Modulo>(data(), rhs);
    return *this;
}

void kF::Var::reserve(const Meta::Type type) noexcept_ndebug
{
    if (type.isTrivial())
        reserve<true>(type);
    else
        reserve<false>(type);
}