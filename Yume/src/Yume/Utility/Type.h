#pragma once

template<typename E>
constexpr std::underlying_type_t<E> toUType(E enumerator) noexcept
{
	return static_cast<std::underlying_type_t<E>>(enumerator);
}
