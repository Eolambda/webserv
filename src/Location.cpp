/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vincentfresnais <vincentfresnais@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 16:13:39 by wouhliss          #+#    #+#             */
/*   Updated: 2025/02/13 16:55:58 by vincentfres      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Location.hpp>

Location::Location()
{
	_route = "";
	_allowed_methods = 0;
	_directory_listing = false;
	_path = "";
	_redirect = "";
	_index = "";
}

Location::~Location()
{
}

void Location::addAllowedMethod(const std::string &value)
{

	_allowed_methods = 0;

	std::string method;
	std::string copy = value;

	while (copy.size() > 0)
	{
		if (copy.find(",") != std::string::npos)
		{
			method = copy.substr(0, copy.find(","));
			copy.erase(0, copy.find(",") + 1);
		}
		else
		{
			method = copy;
			copy.clear();
		}

		method = trim_spaces(method);

		if (method == "GET")
		{
			_allowed_methods |= 0x1;
		}
		else if (method == "POST")
		{
			_allowed_methods |= 0x2;
		}
		else if (method == "DELETE")
		{
			_allowed_methods |= 0x4;
		}
	}
}

void Location::setPath(const std::string &value)
{
	_path = value;
}

void Location::setRedirect(const std::string &value)
{
	_redirect = value;
}

void Location::setDirectoryListing(const bool value)
{
	_directory_listing = value;
}

void Location::setIndex(const std::string &value)
{
	_index = value;
}

void Location::setRoute(const std::string &value)
{
	_route = value;
}

uint8_t Location::getAllowedMethods(void) const
{
	return (_allowed_methods);
}

std::string &Location::getPath(void)
{
	return (_path);
}

std::string &Location::getRedirect(void)
{
	return (_redirect);
}

bool Location::getDirectoryListing(void) const
{
	return (_directory_listing);
}

std::string &Location::getIndex(void)
{
	return (_index);
}

std::string &Location::getRoute(void)
{
	return (_route);
}