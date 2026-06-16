#include "test_runner.hpp"
#include "Clients/Client.hpp"
#include "Clients/Clients.hpp"

TEST(clients_add_returns_zero)
{
    Clients container;
    Client  client(42);
    client.setNickname("alice");
    ASSERT_EQ(container.add_client(client), 0);
}

TEST(clients_add_duplicate_fd_returns_nonzero)
{
    Clients container;
    Client  a(42);
    a.setNickname("alice");
    Client  b(42);
    b.setNickname("bob");
    container.add_client(a);
    ASSERT_EQ(container.add_client(b), 1);
}

TEST(clients_get_existing_returns_pointer)
{
    Clients container;
    Client  client(42);
    client.setNickname("alice");
    container.add_client(client);
    ASSERT(container.get_client("alice") != NULL);
}

TEST(clients_get_returns_correct_fd)
{
    Clients container;
    Client  client(42);
    client.setNickname("alice");
    container.add_client(client);
    ASSERT_EQ(container.get_client("alice")->getFd(), 42);
}

TEST(clients_get_missing_returns_null)
{
    Clients container;
    ASSERT(container.get_client("nobody") == NULL);
}

TEST(clients_get_without_nickname_returns_null)
{
    Clients container;
    Client  client(42);
    container.add_client(client);
    ASSERT(container.get_client("") == NULL);
}

TEST(clients_remove_existing_returns_zero)
{
    Clients container;
    Client  client(42);
    client.setNickname("alice");
    container.add_client(client);
    ASSERT_EQ(container.remove_client("alice"), 0);
}

TEST(clients_remove_missing_returns_nonzero)
{
    Clients container;
    ASSERT_EQ(container.remove_client("nobody"), 1);
}

TEST(clients_get_after_remove_returns_null)
{
    Clients container;
    Client  client(42);
    client.setNickname("alice");
    container.add_client(client);
    container.remove_client("alice");
    ASSERT(container.get_client("alice") == NULL);
}

TEST(clients_multiple_independent_clients)
{
    Clients container;
    Client  a(1);
    a.setNickname("alice");
    Client  b(2);
    b.setNickname("bob");
    container.add_client(a);
    container.add_client(b);
    ASSERT(container.get_client("alice") != NULL);
    ASSERT(container.get_client("bob") != NULL);
    container.remove_client("alice");
    ASSERT(container.get_client("alice") == NULL);
    ASSERT(container.get_client("bob") != NULL);
}

TEST(clients_remove_by_fd_returns_zero)
{
    Clients container;
    Client  client(42);
    client.setNickname("alice");
    container.add_client(client);
    ASSERT_EQ(container.remove_client(42), 0);
}

TEST(clients_remove_by_fd_missing_returns_nonzero)
{
    Clients container;
    ASSERT_EQ(container.remove_client(99), 1);
}

TEST(clients_remove_by_fd_clears_nick_lookup)
{
    Clients container;
    Client  client(42);
    client.setNickname("alice");
    container.add_client(client);
    container.remove_client(42);
    ASSERT(container.get_client("alice") == NULL);
    ASSERT(container.get_client_fd(42) == NULL);
}
