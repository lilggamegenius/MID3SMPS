#pragma once

#include <utility>
#include <optional>

template<typename T, bool autoDirty = false>
class dirtyable{
	std::optional<T> data = std::nullopt;
	bool isDirty = true;

public:
	constexpr dirtyable() = default;

	constexpr explicit dirtyable(T&& newData){
		data = std::move(newData);
	}

	template<typename ...Args>
	constexpr explicit dirtyable(Args&& ...args){
		data = T(std::forward<Args>(args)...);
	}

	constexpr explicit dirtyable(const T &newData){
		data = newData;
	}

	constexpr void markDirty(){
		isDirty = true;
	}

	constexpr void clearDirty(){
		isDirty = false;
	}

	[[nodiscard]] constexpr bool dirty() const {
		return isDirty;
	}

	constexpr T& operator *(){
		if constexpr (autoDirty){
			isDirty = true;
		}
		return *data;
	}

	constexpr const T& operator *() const{
		return *data;
	}

	constexpr T* operator->(){
		if constexpr(autoDirty){
			isDirty = true;
		}
		return data.operator->();
	}

	constexpr const T* operator->() const{
		return data.operator->();
	}

	constexpr dirtyable& operator =(T&& newData){
		if(data != newData){
			isDirty = true;
		}
		data = std::make_optional<T>(std::move(newData));
		return *this;
	}

	constexpr dirtyable& operator =(const T &newData){
		if(data != newData){
			data = newData;
			isDirty = true;
		}
		return *this;
	}

	constexpr dirtyable(dirtyable&& other) noexcept {
		if(*other != *data){
			isDirty = other.isDirty;
		}
		data = std::move(other.data);
	}

	constexpr dirtyable(const dirtyable &other){
		if(*other != *data){
			isDirty = other.isDirty;
		}
		data = other.data;
	}

	constexpr auto operator <=>(const T &otherData) const{
		return *data <=> otherData;
	}

	constexpr auto operator <=>(const dirtyable &otherData) const {
		return *data <=> *otherData;
	}

	constexpr operator bool() const noexcept {
		return data.operator bool();
	}

	constexpr auto has_value() const noexcept {
		return data.has_value();
	}
};
