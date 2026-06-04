/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmurua <tmurua@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 17:43:29 by tmurua            #+#    #+#             */
/*   Updated: 2026/06/04 18:44:20 by tmurua           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server {
private:
	int			_port;
	std::string	_password;

	int		parsePort(const std::string& port) const;
	bool	isOnlyDigits(const std::string& text) const;

public:
	Server(const std::string& port, const std::string& password);
	~Server();
};

#endif
