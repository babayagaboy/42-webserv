/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 17:23:58 by hgutterr          #+#    #+#             */
/*   Updated: 2026/07/11 15:15:07 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "iter.hpp"
#include <iostream>

template <typename V>
void increment1(V& value) {
	value += 1;
}

template <typename V>
void print1(const V& value) {
	std::cout << "P: " << value << std::endl;
}

void to_upper_re(std::string& str) {
    for(size_t i = 0; i < str.size(); ++i){
        str[i] = static_cast<char>(std::toupper(static_cast<char>(str[i])));
    };
}

int main()
{
	{	
		int arr[] = {1, 2, 3};
		ITER_HPP::iter(arr, 3 , increment1<int>);
		for (int n = 0; n < 3; ++n) {
			std::cout << "arr[" << n << "]: " << arr[n] << std::endl;
		}
	}
	{
		char arr[] = {'a', 'b', 'c'};
		ITER_HPP::iter(arr, 3 , print1<const char>);
	}
	{
		std::string arr[] = {"eto", "khuya", "pider"};
		ITER_HPP::iter(arr, 3 , to_upper_re);
		for (int n = 0; n < 3; ++n) {
			std::cout << "arr[" << n << "]: " << arr[n] << std::endl;
		}
	}
}