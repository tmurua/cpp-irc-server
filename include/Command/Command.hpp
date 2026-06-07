/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 16:15:57 by johartma          #+#    #+#             */
/*   Updated: 2026/06/07 19:08:12 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HPP
# define COMMAND_HPP
# include <string>
# include <vector>
# include "CmdTypes.hpp"
# include "Server.hpp"


class Command {
	private:
    	std::string                 _prefix;
    	CmdType::Type               _type;
    	std::vector<std::string>    _arguments;
    	std::string                 _trailing;
	public:	
    	Command(const std::string& raw);
		~Command();
		int	execute(Server& serv);
};

#endif