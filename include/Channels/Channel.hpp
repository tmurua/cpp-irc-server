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
};

#endif