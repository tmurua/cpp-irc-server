/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channels.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/07 19:14:46 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:13:52 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNELS_HPP
# define CHANNELS_HPP
# include <map>
# include <string>
# include "Channel.hpp"

class Channels {
	private:
		std::map<std::string, Channel>	channels;
	public:
		Channels();
		~Channels();
		int			add_channel(Channel &chan);
		int			remove_channel(std::string name);
		Channel*	get_channel(std::string name);
		const std::map<std::string, Channel>& getAll() const { return channels; }
};
# endif