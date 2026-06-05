/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 16:15:57 by johartma          #+#    #+#             */
/*   Updated: 2026/06/05 18:49:52 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HPP
# define COMMAND_HPP
# include <string>
# include <vector>
# include "CmdTypes.hpp"


struct Command {
    std::string                 _prefix;
    CmdType::Type               _type;
    std::vector<std::string>    _arguments;
    std::string                 _trailing;
    Command(const std::string& raw);
};

#endif