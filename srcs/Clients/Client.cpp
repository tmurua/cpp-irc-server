/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 00:00:00 by johartma          #+#    #+#             */
/*   Updated: 2026/06/09 00:00:00 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Clients/Client.hpp"
#include <string>

Client::Client(int fd)
    : _fd(fd), _nickname(""), _username(""), _realname(""),
      _hostname(""), _server(""), _registered(false), _authenticated(false), _isop(false) {}

Client::~Client() {}

int Client::getFd() const { return _fd; }
const std::string& Client::getNickname() const { return _nickname; }
const std::string& Client::getUsername() const { return _username; }
const std::string& Client::getRealname() const { return _realname; }
const std::string& Client::getHostname() const { return _hostname; }
const std::string& Client::getServer() const { return _server; }
bool Client::isRegistered() const { return _registered; }
bool Client::isOp() const { return _isop; }
const std::set<std::string>& Client::getChannels() const { return _channels; }

int Client::setNickname(const std::string& nickname) {
    if (nickname.length() > 9)
        return (1);
    _nickname = nickname;
    return (0);
}

void Client::setUsername(const std::string& username) { _username = username; }
void Client::setRealname(const std::string& realname) { _realname = realname; }
void Client::setHostname(const std::string& hostname) { _hostname = hostname; }
void Client::setServer(const std::string& server) { _server = server; }
void Client::setRegistered(bool registered) { _registered = registered; }
void Client::setOp(bool isop) { _isop = isop; }
bool Client::setAuthenticated(bool authenticated) {_authenticated = authenticated; };

void Client::joinChannel(const std::string& channel) { _channels.insert(channel); }

void Client::leaveChannel(const std::string& channel) { _channels.erase(channel); }

bool Client::inChannel(const std::string& channel) const {
    return _channels.count(channel) > 0;
}
