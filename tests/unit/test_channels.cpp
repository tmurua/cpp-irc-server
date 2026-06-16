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

TEST(channel_topic_restricted_default_false)
{
    Channel chan("#test");
    ASSERT(!chan.isTopicRestricted());
}

TEST(channel_set_topic_restricted)
{
    Channel chan("#test");
    chan.setTopicRestricted(true);
    ASSERT(chan.isTopicRestricted());
    chan.setTopicRestricted(false);
    ASSERT(!chan.isTopicRestricted());
}

TEST(channel_invite_list_add_and_query)
{
    Channel chan("#test");
    ASSERT(!chan.isInvited("alice"));
    chan.addInvite("alice");
    ASSERT(chan.isInvited("alice"));
    ASSERT(!chan.isInvited("bob"));
}

TEST(channel_invite_clear)
{
    Channel chan("#test");
    chan.addInvite("alice");
    chan.clearInvite("alice");
    ASSERT(!chan.isInvited("alice"));
}

TEST(channel_invite_independent_of_users)
{
    Channel chan("#test");
    chan.addUser("alice");
    chan.addInvite("bob");
    ASSERT(chan.hasUser("alice"));
    ASSERT(!chan.hasUser("bob"));
    ASSERT(chan.isInvited("bob"));
}
