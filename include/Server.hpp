/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 17:43:29 by tmurua            #+#    #+#             */
/*   Updated: 2026/06/05 16:24:13 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <map>


class Server {
private:
	int			                    _port;
	std::string                     _password;

	int		parsePort(const std::string& port) const;
	bool	isOnlyDigits(const std::string& text) const;

public:
	Server(const std::string& port, const std::string& password);
	~Server();
};

# endif
