#pragma once

#include <memory>

namespace safeout
{
  namespace details
  {
    struct _unused {};
    inline _unused _;
  }

  // This is a basic (and lazy) implementation to make use
  // of already existing code (std::unique_ptr)
  // ex of other impl: StackUnwinder https://github.com/latimagine/statismo/blob/master/modules/core/include/statismo/core/CommonTypes.h
  template <typename Callable>
  auto make_guard(Callable c)
  {
    auto deleter = [del = std::move(c)](details::_unused*)
    {
      del();
    };
    return std::unique_ptr<details::_unused, decltype(deleter)>(&details::_, deleter);
  }

  template <typename T, typename Deleter = std::default_delete<T>>
  auto make_guard(T* ptr, Deleter && del = Deleter {})
  {
    return std::unique_ptr<T, Deleter>(ptr, std::forward<Deleter>(del));
  }
}

// List of macros

#define _safeout_push_at(val, args...) auto _safeout_##val = safeout::make_guard(args);
#define _safeout_push_0(args...) _safeout_push_at(0, args)
#define _safeout_push_1(args...) _safeout_push_at(1, args)
#define _safeout_push_2(args...) _safeout_push_at(2, args)
#define _safeout_push_3(args...) _safeout_push_at(3, args)
#define _safeout_push_4(args...) _safeout_push_at(4, args)
#define _safeout_push_5(args...) _safeout_push_at(5, args)
#define _safeout_push_6(args...) _safeout_push_at(6, args)
#define _safeout_push_7(args...) _safeout_push_at(7, args)
#define _safeout_push _safeout_push_0

#define _safeout_pop_at(val) _safeout_##val.release();
#define _safeout_pop_0() _safeout_pop_at(0)
#define _safeout_pop_1() _safeout_pop_at(1)
#define _safeout_pop_2() _safeout_pop_at(2)
#define _safeout_pop_3() _safeout_pop_at(3)
#define _safeout_pop_4() _safeout_pop_at(4)
#define _safeout_pop_5() _safeout_pop_at(5)
#define _safeout_pop_6() _safeout_pop_at(6)
#define _safeout_pop_7() _safeout_pop_at(7)
#define _safeout_pop() _safeout_pop_0()

#define _safeout_get_at(val) _safeout_##val.get();
#define _safeout_get_0() _safeout_get_at(0)
#define _safeout_get_1() _safeout_get_at(1)
#define _safeout_get_2() _safeout_get_at(2)
#define _safeout_get_3() _safeout_get_at(3)
#define _safeout_get_4() _safeout_get_at(4)
#define _safeout_get_5() _safeout_get_at(5)
#define _safeout_get_6() _safeout_get_at(6)
#define _safeout_get_7() _safeout_get_at(7)
#define _safeout_get() _safeout_get_0()
