/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 16:16:03 by johartma          #+#    #+#             */
/*   Updated: 2026/06/07 22:19:22 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP
# include <string>
# include <set>

class Client {
    private:
        int                     _fd;
        std::string             _nickname; //max 9 characters
        std::string             _hostname;
        std::string             _server;
        bool                    _registered;
        bool                    _isop;
        std::set<std::string>   _channels;       
    
    public:
        Client(int fd);
        ~Client();
};

#endif