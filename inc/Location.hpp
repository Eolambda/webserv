/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vincentfresnais <vincentfresnais@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 15:50:59 by wouhliss          #+#    #+#             */
/*   Updated: 2025/02/13 16:43:42 by vincentfres      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __LOCATION_HPP__
#define __LOCATION_HPP__

#include <webserv.hpp>

class Location
{
public:
	Location();
	~Location();

	void addAllowedMethod(const std::string &value);
	void setPath(const std::string &value);
	void setRedirect(const std::string &value);
	void setDirectoryListing(const bool value);
	void setIndex(const std::string &value);
	void setRoute(const std::string &value);

	uint8_t getAllowedMethods(void) const;
	std::string &getPath(void);
	std::string &getRedirect(void);
	bool getDirectoryListing(void) const;
	std::string &getIndex(void);
	std::string &getRoute(void);

private:
	std::string _path;
	uint8_t _allowed_methods;
	bool _directory_listing;
	std::string _redirect;
	std::string _index;
	std::string _route;
};
#endif
