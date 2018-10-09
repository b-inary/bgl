
// https://rhysd.hatenablog.com/entry/2014/01/28/235958

/**
 * @brief easy lambda wrapper. ex) lambda(a, b) -> [&](const auto &a, const auto &b)<br>
 *        supports up to 8 arguments. does not work on MSVC...
 * @param ... arguments
 */
#define lambda(...) BGL_LAMBDA_III(BGL_IS_EMPTY(__VA_ARGS__))(__VA_ARGS__)


#define BGL_HAS_COMMA_I(a0, a1, a2, a3, a4, a5, a6, a7, ...) a7
#define BGL_HAS_COMMA(...) BGL_HAS_COMMA_I(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 0)
#define BGL_TRIGGER_PARENTHESIS(...) ,

#define BGL_IS_EMPTY(...) \
BGL_IS_EMPTY_I(BGL_HAS_COMMA(__VA_ARGS__), \
               BGL_HAS_COMMA(BGL_TRIGGER_PARENTHESIS __VA_ARGS__), \
               BGL_HAS_COMMA(__VA_ARGS__ (/*empty*/)), \
               BGL_HAS_COMMA(BGL_TRIGGER_PARENTHESIS __VA_ARGS__ (/*empty*/)))

#define BGL_CAT5(a0, a1, a2, a3, a4) a0 ## a1 ## a2 ## a3 ## a4
#define BGL_IS_EMPTY_I(a0, a1, a2, a3) BGL_HAS_COMMA(BGL_CAT5(BGL_IS_EMPTY_CASE_, a0, a1, a2, a3))
#define BGL_IS_EMPTY_CASE_0001 ,

#define BGL_VARIADIC_SIZE(...) BGL_VARIADIC_SIZE_I(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1,)
#define BGL_VARIADIC_SIZE_I(a0, a1, a2, a3, a4, a5, a6, a7, size, ...) size

#define BGL_LAMBDA_8(a, ...)  auto const& a, BGL_LAMBDA_7(__VA_ARGS__)
#define BGL_LAMBDA_7(a, ...)  auto const& a, BGL_LAMBDA_6(__VA_ARGS__)
#define BGL_LAMBDA_6(a, ...)  auto const& a, BGL_LAMBDA_5(__VA_ARGS__)
#define BGL_LAMBDA_5(a, ...)  auto const& a, BGL_LAMBDA_4(__VA_ARGS__)
#define BGL_LAMBDA_4(a, ...)  auto const& a, BGL_LAMBDA_3(__VA_ARGS__)
#define BGL_LAMBDA_3(a, ...)  auto const& a, BGL_LAMBDA_2(__VA_ARGS__)
#define BGL_LAMBDA_2(a, ...)  auto const& a, BGL_LAMBDA_1(__VA_ARGS__)
#define BGL_LAMBDA_1(a)       auto const& a

#define BGL_LAMBDA_III(b) BGL_LAMBDA_IIII(b)
#define BGL_LAMBDA_IIII(b) BGL_LAMBDA_ARGS_##b
#define BGL_LAMBDA_ARGS_1 BGL_LAMBDA_0
#define BGL_LAMBDA_ARGS_0(...) BGL_LAMBDA_0(BGL_LAMBDA_I(BGL_VARIADIC_SIZE(__VA_ARGS__))(__VA_ARGS__))
#define BGL_LAMBDA_I(n) BGL_LAMBDA_II(n)
#define BGL_LAMBDA_II(size) BGL_LAMBDA_##size
#define BGL_LAMBDA_0 [&]
