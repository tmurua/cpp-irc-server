/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 16:15:57 by johartma          #+#    #+#             */
/*   Updated: 2026/06/08 09:58:49 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HPP
# define COMMAND_HPP
# include <string>
# include <vector>
# include "Command/CmdTypes.hpp"
# include <exception>

class Server;

class Command {
	private:
    	std::string                 _prefix;
    	CmdType::Type               _type;
    	std::vector<std::string>    _arguments;
    	std::string                 _trailing;
		int							_fd;
	public:
	  	class MalformedException : public std::exception {
            public:
                const char* what() const throw() {
                    return "Malformed IRC message";
                }
			};
    	Command(int fd, const std::string& raw);
		~Command();
		int	execute(Server& serv);

		int								getFd() const;
		const std::string&				getPrefix() const;
		CmdType::Type					getType() const;
		const std::vector<std::string>&	getArguments() const;
		const std::string&				getTrailing() const;
};

#endif
