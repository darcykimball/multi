#ifndef MULTI_HPP
#define MULTI_HPP


#include <functional>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <typeindex>
#include <map>

#include <boost/hana.hpp>

#include "util.hpp"


namespace detail {

// Here we go
namespace hana = boost::hana;


// Get all possible argument-type combinations
// XXX: All supplied types must be distinct
// TODO: use some sort of canonical ordering, etc.
// FIXME: wait, just use hana::cartesian_product
template <typename Tuple, size_t N>
constexpr auto all_combos(Tuple&& types) {
  auto combos = hana::tuple{};

  for (size_t i; i < N; ++i) {
    
  }
}



// These are expected to be hana type constants.
template <typename ...Ts>
auto get_type_indices(Ts&& ...types) {
  return std::make_tuple(std::type_index(typeid(decltype(types)::type))...);
}


} // namespace detail


namespace multi {

  
  namespace hana = boost::hana;


// The multimethod dispatcher functor (c++), assuming:
//   -all parameters are subclasses of (at least one) base
// TODO: specialization for when CTTI (https://github.com/Manu343726/ctti)
// is available
template <
  typename Base,
  size_t Arity,
  typename Ret,
  template <typename ...> class Impl,
  bool IsTotal,
  typename ...Ts
  > 
class multi_dispatcher {
  // Helper type-level const
  template <typename>
  using always_base = Base;


  // The dispatch table
  using map_type = std::map<
    std::tuple<util::always_type<std::type_index, Ts>...>,
    Ret(*)(always_base<Ts>&...)
    >;


  map_type _dispatch_table;

    

  // Helper for wrapping functions for insertion into map
  template <typename ...Args>
  static constexpr auto wrapped = [](always_base<Args>&... args) {
    return Impl<Args...>::operator()(static_cast<Args&>(args)...);
  };


  // Get ths wrappede instantiation/implementation for a particular overload
  template <typename ...Types>
  static constexpr auto get_impl(Types&& ...types) {
    return wrapped<decltype(types)::type...>;
  }

    
public: 
  multi_dispatcher() {
    // Setup dispatch table
    constexpr auto combos = detail::all_combos<Arity>(hana::tuple_t<Ts...>);
    
    hana::transform(combos, [*this](auto&& c) {
      this->_dispatch_table.emplace(
        std::make_pair(
          hana::unpack(c, detail::get_type_indices),
          // FIXME/TODO: what's the cleanest way to do this???
          hana::unpack(c, get_impl)
        )
      ); 
    });
  }


  template <typename ...Unused>
  Ret operator()(always_base<Unused>& ...args) {
    // XXX: Hopefully this is the sanest place to induce an error
    static_assert(sizeof...(Unused) == Arity,
      "Arity mismatch for multimethod call");

   
    // TODO: (Statically) check that the supplied template params are a
    // subset of the type set

    
    // Get runtime types of arguments
    auto types_key = std::make_tuple(std::type_index(typeid(args))...);
    
    // Lookup combo in table
    auto found = _dispatch_table.find(types_key);


    // FIXME: how to guarantee this'll be optimized out? probably will be?
    if (IsTotal) {
      return found->second(args...);
    }


    if (found != _dispatch_table.end()) {
      return found->second(args...);
    }

    // Didn't find it.
    throw std::bad_function_call{};
  }
};


} // namespace multi



#endif // MULTI_HPP
