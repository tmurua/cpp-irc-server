/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 16:16:03 by johartma          #+#    #+#             */
/*   Updated: 2026/06/05 17:48:41 by johartma         ###   ########.fr       */
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
        Client();
        ~Client();
};

#endif