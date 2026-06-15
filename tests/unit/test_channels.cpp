/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_channels.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 00:00:00 by johartma          #+#    #+#             */
/*   Updated: 2026/06/09 00:00:00 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test_runner.hpp"
#include "Channels/Channel.hpp"
#include "Channels/Channels.hpp"

TEST(channels_add_returns_zero)
{
    Channels container;
    Channel  chan("#general");
    ASSERT_EQ(container.add_channel(chan), 0);
}

TEST(channels_add_duplicate_returns_nonzero)
{
    Channels container;
    Channel  chan("#general");
    container.add_channel(chan);
    ASSERT_EQ(container.add_channel(chan), 1);
}

TEST(channels_get_existing_returns_pointer)
{
    Channels container;
    Channel  chan("#general");
    container.add_channel(chan);
    ASSERT(container.get_channel("#general") != NULL);
}

TEST(channels_get_returns_correct_name)
{
    Channels container;
    Channel  chan("#general");
    container.add_channel(chan);
    ASSERT_EQ(container.get_channel("#general")->getName(), std::string("#general"));
}

TEST(channels_get_missing_returns_null)
{
    Channels container;
    ASSERT(container.get_channel("#missing") == NULL);
}

TEST(channels_remove_existing_returns_zero)
{
    Channels container;
    Channel  chan("#general");
    container.add_channel(chan);
    ASSERT_EQ(container.remove_channel("#general"), 0);
}

TEST(channels_remove_missing_returns_nonzero)
{
    Channels container;
    ASSERT_EQ(container.remove_channel("#missing"), 1);
}

TEST(channels_get_after_remove_returns_null)
{
    Channels container;
    Channel  chan("#general");
    container.add_channel(chan);
    container.remove_channel("#general");
    ASSERT(container.get_channel("#general") == NULL);
}

TEST(channels_multiple_independent_channels)
{
    Channels container;
    Channel  a("#general");
    Channel  b("#random");
    container.add_channel(a);
    container.add_channel(b);
    ASSERT(container.get_channel("#general") != NULL);
    ASSERT(container.get_channel("#random") != NULL);
    container.remove_channel("#general");
    ASSERT(container.get_channel("#general") == NULL);
    ASSERT(container.get_channel("#random") != NULL);
}
