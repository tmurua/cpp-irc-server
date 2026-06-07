/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 17:43:29 by tmurua            #+#    #+#             */
/*   Updated: 2026/06/07 19:14:01 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include "Client.hpp"


class Server {
private:
	int			                    _port;
	std::string                     _password;

	int		parsePort(const std::string& port) const;
	bool	isOnlyDigits(const std::string& text) const;

public:
	Server(const std::string& port, const std::string& password);
	~Server();
	int		executecmd(const std::string& raw);
	int		sndmsg(const Client& clnt, const std::string& msg);
};

# endif
