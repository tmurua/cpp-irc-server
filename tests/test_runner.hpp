#ifndef TEST_RUNNER_HPP
# define TEST_RUNNER_HPP

# include <iostream>
# include <cstdlib>
# include <cstring>
# include <stdexcept>
# include <vector>

struct TestEntry {
	const char*	name;
	void		(*fn)();
};

inline std::vector<TestEntry>&	get_tests()
{
	static std::vector<TestEntry>	tests;
	return tests;
}

inline void	register_test(const char* name, void (*fn)())
{
	TestEntry	entry;

	entry.name = name;
	entry.fn = fn;
	get_tests().push_back(entry);
}

inline int	run_all_tests()
{
	std::vector<TestEntry>&	tests = get_tests();
	int						passed = 0;
	int						failed = 0;

	for (size_t i = 0; i < tests.size(); ++i) {
		std::cout << "[ RUN  ] " << tests[i].name << std::endl;
		try {
			tests[i].fn();
			std::cout << "[ PASS ] " << tests[i].name << std::endl;
			++passed;
		} catch (...) {
			std::cout << "[ FAIL ] " << tests[i].name << std::endl;
			++failed;
		}
	}

	std::cout << std::endl
			  << passed << " passed, " << failed << " failed" << std::endl;
	return failed == 0 ? 0 : 1;
}

# define TEST(name) \
	static void test_##name(); \
	struct TestRegistrar_##name { \
		TestRegistrar_##name() { register_test(#name, test_##name); } \
	}; \
	static TestRegistrar_##name g_registrar_##name; \
	static void test_##name()

# define ASSERT(cond) \
	do { \
		if (!(cond)) { \
			std::cerr << "  ASSERT failed: " << #cond \
					  << " (" << __FILE__ << ":" << __LINE__ << ")" \
					  << std::endl; \
			throw std::runtime_error("assertion failed"); \
		} \
	} while (0)

# define ASSERT_EQ(a, b) \
	do { \
		if ((a) != (b)) { \
			std::cerr << "  ASSERT_EQ failed: " << #a << " != " << #b \
					  << " (" << __FILE__ << ":" << __LINE__ << ")" \
					  << std::endl; \
			throw std::runtime_error("assertion failed"); \
		} \
	} while (0)

# define ASSERT_STREQ(a, b) \
	do { \
		if (std::strcmp((a), (b)) != 0) { \
			std::cerr << "  ASSERT_STREQ failed: \"" << (a) \
					  << "\" != \"" << (b) << "\"" \
					  << " (" << __FILE__ << ":" << __LINE__ << ")" \
					  << std::endl; \
			throw std::runtime_error("assertion failed"); \
		} \
	} while (0)

# define ASSERT_THROW(expr, exc_type) \
	do { \
		bool threw = false; \
		try { \
			expr; \
		} catch (const exc_type&) { \
			threw = true; \
		} catch (...) { \
			std::cerr << "  ASSERT_THROW failed: wrong exception type" \
					  << " (" << __FILE__ << ":" << __LINE__ << ")" \
					  << std::endl; \
			throw std::runtime_error("assertion failed"); \
		} \
		if (!threw) { \
			std::cerr << "  ASSERT_THROW failed: " << #expr \
					  << " did not throw " << #exc_type \
					  << " (" << __FILE__ << ":" << __LINE__ << ")" \
					  << std::endl; \
			throw std::runtime_error("assertion failed"); \
		} \
	} while (0)

#endif
