/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmurua <tmurua@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 16:56:44 by tmurua            #+#    #+#             */
/*   Updated: 2026/06/04 18:55:40 by tmurua           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <iostream>

int main(int argc, char **argv){
	if (argc != 3){
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return (1);
	}
	try {
		Server server(argv[1], argv[2]);

		server.start();
	}
	catch (const std::exception& error){
		std::cerr << "Error: " << error.what() << std::endl;
		return (1);
	}
	return (0);
}
