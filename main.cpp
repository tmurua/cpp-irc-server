/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmurua <tmurua@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 16:56:44 by tmurua            #+#    #+#             */
/*   Updated: 2026/06/04 17:02:17 by tmurua           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

int main(int argc, char **argv){
	(void)argv;	

	if (argc != 3){
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return (1);
	}
	return (0);
}
