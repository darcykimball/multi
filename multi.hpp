#ifndef MULTI_HPP
#define MULTI_HPP


#include <tuple>
#include <type_traits>

#include <boost/hana.hpp>


namespace detail {

// Here we go
namespace hana = boost::hana;


// Get all possible argument-type combinations
// TODO: use some sort of canonical ordering, etc.
template <typename Tuple>
constexpr auto all_combos(size_t N, Tuple&& types) {
  return types
    | [&](auto&& t) { 
        return hana::plus(
            hana::lift<hana::tuple_tag>(std::forward<decltype(t)>(t)),
            all_combos(N - 1, std::forward<decltype(types)>(types))
          );
      };
}



// These are expected to be hana type constants.
template <typename ...Ts>
auto get_type_indices(Ts&& ...types) {
  return std::make_tuple(typeid(types)...);
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
  using map_type = std::unordered_map<
    std::tuple<util::always_type<std::type_info, Ts>...>,
    Ret(*)(always_base<Args>&...)
    >;


  map_type _dispatch_map;

    

  // Helper for wrapping functions for insertion into map
  template <typename ...Args>
  static auto wrapped = [](always_base<Args>&... args) {
      return Impl<Args...>::operator()(static_cast<Args&>(args)...);
    };
  }

    
public: 
  multi_dispatcher() {
    // Setup dispatch table
    auto const combos = detail::all_combos(hana::tuple_t<Ts...>{});
    
    hana::transform(combos, [&this->_dispatch_map](auto&& c) {
      _dispatch_map.emplace(std::make_pair(
        hana::unpack(c, detail::get_type_indices),
        // FIXME/TODO: what's the cleanest way to do this???        
    });

  }


  template <typename ...Unused>
  std::enable_if_t<IsTotal, Ret> // XXX: This overload assumes totality
  operator()(always_base<Unused>& ...args) {
    // XXX: Hopefully this is the sanest place to induce an error
    static_assert(sizeof...(Unused) == Arity,
      "Arity mismatch for multimethod call");

   
    // TODO: (Statically) check that the supplied template params are a
    // subset of the type set

    
    // Get runtime types of arguments
    auto types_key = std::make_tuple(std::type_index(typeid(args))...);
    
    // Lookup combo in table
    auto found = _dispatch_table.find(types_key);
    return found->second(args...);
  }


  // TODO:
  template <typename ...Unused>
  std::enable_if_t<!IsTotal, Ret> // XXX: This overload assumes partiality
  operator()(always_base<Unused>& ...args) {
    // Get runtime types of arguments
    auto types_key = std::make_tuple(std::type_index(typeid(args))...);
    
    // Lookup combo in table
    auto found = _dispatch_table.find(types_key);
    if (found != _dispatch_table.end()) {
      return found->second(args...);
    }

    // Didn't find it.
    throw std::bad_function_call{};
  }
};


} // namespace multi



#endif // MULTI_HPP
