/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 16:16:09 by johartma          #+#    #+#             */
/*   Updated: 2026/06/07 22:19:36 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP
# include <string>
# include <set>

class Channel {
    private:
        std::string             _name; //max 200 chars, start with & or #, space, ^G or ",""
        bool                    _invite_only;
        bool                    _has_key;
        std::string             _key;
        std::string             _topic;
        bool                    _has_topic;
        std::set<std::string>   _chan_users;
        std::set<std::string>   _chan_ops;
        int                     _user_limit;
    public:
        Channel(std::string name);
        ~Channel();

        // Getters
        const std::string&              getName() const;
        bool                            isInviteOnly() const;
        bool                            hasKey() const;
        const std::string&              getKey() const;
        bool                            hasTopic() const;
        const std::string&              getTopic() const;
        const std::set<std::string>&    getUsers() const;
        const std::set<std::string>&    getOps() const;
        int                             getUserLimit() const;

        // Setters
        void    setInviteOnly(bool invite_only);
        void    setKey(const std::string& key);
        void    clearKey();
        void    setTopic(const std::string& topic);
        void    clearTopic();
        void    setUserLimit(int limit);

        // User management
        void    addUser(const std::string& nick);
        void    removeUser(const std::string& nick);
        bool    hasUser(const std::string& nick) const;

        // Operator management
        void    addOp(const std::string& nick);
        void    removeOp(const std::string& nick);
        bool    isOp(const std::string& nick) const;
};

#endif