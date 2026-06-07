/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Clients.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/07 19:23:21 by johartma          #+#    #+#             */
/*   Updated: 2026/06/07 19:26:58 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTS_HPP
# define CLIENTS_HPP
# include <map>
# include <string>
# include "Client.hpp"

class Clients {
	private:
		std::map<std::string, Client>	channels;
	public:
		Clients();
		~Clients();
		int			update_client(Client &clie);
		int			remove_client(std::string name);
		Client*		get_client(std::string name);
};
# endif