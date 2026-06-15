/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channels.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/15 18:14:30 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:14:34 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include "Channels/Channels.hpp"

Channels::Channels() {}

Channels::~Channels() {}

int Channels::add_channel(Channel& chan) {
    if (channels.count(chan.getName()))
        return 1;
    channels.insert(std::make_pair(chan.getName(), chan));
    return 0;
}

int Channels::remove_channel(std::string name) {
    return channels.erase(name) > 0 ? 0 : 1;
}

Channel* Channels::get_channel(std::string name) {
    std::map<std::string, Channel>::iterator it = channels.find(name);
    if (it == channels.end())
        return NULL;
    return &it->second;
}
