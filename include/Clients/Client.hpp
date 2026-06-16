/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 16:16:03 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:13:57 by johartma         ###   ########.fr       */
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
        std::string             _username;
        std::string             _realname;
        std::string             _hostname;
        std::string             _server;
        bool                    _authenticated;
        bool                    _registered;
        bool                    _isop;
        std::set<std::string>   _channels;       
    
    public:
        Client(int fd);
        ~Client();

        // Getters
        int                             getFd() const;
        const std::string&              getNickname() const;
        const std::string&              getUsername() const;
        const std::string&              getRealname() const;
        const std::string&              getHostname() const;
        const std::string&              getServer() const;
        bool                            isRegistered() const;
        bool                            isOp() const;
        bool                            isAuthenticated() const;
        const std::set<std::string>&    getChannels() const;

        // Setters
        int     setNickname(const std::string& nickname);
        void    setUsername(const std::string& username);
        void    setRealname(const std::string& realname);
        void    setHostname(const std::string& hostname);
        void    setServer(const std::string& server);
        void    setRegistered(bool registered);
        bool    setAuthenticated(bool authenticated);

        void    setOp(bool isop);

        // Channel membership
        void    joinChannel(const std::string& channel);
        void    leaveChannel(const std::string& channel);
        bool    inChannel(const std::string& channel) const;
};

#endif