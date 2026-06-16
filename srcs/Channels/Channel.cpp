/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/15 16:33:46 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:14:16 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include "Channels/Channel.hpp"

Channel::Channel(std::string name)
    : _name(name), _invite_only(false), _topic_restricted(false), _has_key(false),
      _key(""), _topic(""), _has_topic(false), _user_limit(0) {}

Channel::~Channel() {}

const std::string& Channel::getName() const { return _name; }
bool Channel::isInviteOnly() const { return _invite_only; }
bool Channel::hasKey() const { return _has_key; }
const std::string& Channel::getKey() const { return _key; }
bool Channel::hasTopic() const { return _has_topic; }
const std::string& Channel::getTopic() const { return _topic; }
const std::set<std::string>& Channel::getUsers() const { return _chan_users; }
const std::set<std::string>& Channel::getOps() const { return _chan_ops; }
int Channel::getUserLimit() const { return _user_limit; }

bool Channel::isTopicRestricted() const { return _topic_restricted; }

void Channel::setInviteOnly(bool invite_only) { _invite_only = invite_only; }
void Channel::setTopicRestricted(bool restricted) { _topic_restricted = restricted; }

void Channel::setKey(const std::string& key) {
    _key = key;
    _has_key = true;
}

void Channel::clearKey() {
    _key = "";
    _has_key = false;
}

void Channel::setTopic(const std::string& topic) {
    _topic = topic;
    _has_topic = true;
}

void Channel::clearTopic() {
    _topic = "";
    _has_topic = false;
}

void Channel::setUserLimit(int limit) { _user_limit = limit; }

void Channel::addUser(const std::string& nick) { _chan_users.insert(nick); }

void Channel::removeUser(const std::string& nick) {
    _chan_users.erase(nick);
    _chan_ops.erase(nick);
}

bool Channel::hasUser(const std::string& nick) const {
    return _chan_users.count(nick) > 0;
}

void Channel::addOp(const std::string& nick) { _chan_ops.insert(nick); }

void Channel::removeOp(const std::string& nick) { _chan_ops.erase(nick); }

bool Channel::isOp(const std::string& nick) const {
    return _chan_ops.count(nick) > 0;
}

void Channel::addInvite(const std::string& nick) { _invite_list.insert(nick); }
bool Channel::isInvited(const std::string& nick) const { return _invite_list.count(nick) > 0; }
void Channel::clearInvite(const std::string& nick) { _invite_list.erase(nick); }
