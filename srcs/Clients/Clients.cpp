/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Clients.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 00:00:00 by johartma          #+#    #+#             */
/*   Updated: 2026/06/09 00:00:00 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Clients/Clients.hpp"

Clients::Clients() {}

Clients::~Clients() {}

int Clients::add_client(Client& client) {
    if (clients.count(client.getFd()))
        return 1;
    clients.insert(std::make_pair(client.getFd(), client));
    if (!client.getNickname().empty())
        lut.insert(std::make_pair(client.getNickname(), client.getFd()));
    return 0;
}

int Clients::update_nick(int fd, std::string nick) {
    if (lut.count(nick))
        return (3);  
    Client* cl = this->get_client_fd(fd);
    if (!cl)
        return (2);
    std::string oldnick = cl->getNickname();
    if (cl->setNickname(nick))
        return (1);
    if (!oldnick.empty())
        lut.erase(oldnick);
    lut[nick] = fd;
    return (0);
}

int Clients::remove_client(int fd) {   
    Client*  cl = this->get_client_fd(fd);
    if (!cl)
        return (1);
    lut.erase(cl->getNickname());
    clients.erase(fd);
    return 0;
}

Client* Clients::get_client_fd(int fd) {

    std::map<int, Client>::iterator cit = clients.find(fd);
    if (cit == clients.end())
        return NULL;
    return &cit->second;
}

Client* Clients::get_client(std::string name) {
    std::map<std::string, int>::iterator it = lut.find(name);
    if (it == lut.end())
        return NULL;
    return (get_client_fd(it->second));
}


