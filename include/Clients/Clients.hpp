/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Clients.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/07 19:23:21 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:13:59 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTS_HPP
# define CLIENTS_HPP
# include <map>
# include <string>
# include "Client.hpp"
#include "Command/Command.hpp"

class Clients {
	private:
		std::map<int, Client>	clients;
		std::map<std::string, int> lut;
	public:
		Clients();
		~Clients();
		int			add_client(Client &client);
		int			update_nick(int fd, std::string nick);
		int			remove_client(int fd);
		int			remove_client(std::string nick);
		Client*		get_client(std::string name);
		Client*		get_client_fd(int fd);

};
# endif