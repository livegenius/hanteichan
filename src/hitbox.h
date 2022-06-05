#ifndef HITBOX_H_GUARD
#define HITBOX_H_GUARD
#include <map>

struct Hitbox
{
	int xy[4];
};

template <template <typename> class Allocator = std::allocator>
class BoxList_T : public std::map<int, Hitbox, std::less<int>, Allocator<std::pair<const int, Hitbox>>>
{
public:
	template<template<typename> class FromT>
	BoxList_T<Allocator>& operator=(const BoxList_T<FromT>& from) {
		clear();
		for(auto it = from.begin(); it != from.end(); it++)
			operator[](it->first) = it->second;
		return *this;
	}
};

using BoxList = BoxList_T<>;

#endif /* HITBOX_H_GUARD */
