#ifndef MULTI_HPP
#define MULTI_HPP


#include <functional>
#include <iostream> // FIXME
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <typeindex>
#include <map>

#include <boost/hana.hpp>
#include <boost/hana/experimental/printable.hpp>

#include "util.hpp"


// FIXME
struct object;
struct ship;
struct shot;


namespace detail {

// Here we go
namespace hana = boost::hana;


// Get all possible argument-type combinations
// XXX: All supplied types must be distinct
// TODO: use some sort of canonical ordering, etc.
// FIXME: wait, just use hana::cartesian_product
template <typename Tuple, typename Size>
constexpr auto all_combos(Tuple&& types, Size&& n) {
  return hana::cartesian_product(hana::replicate<hana::tuple_tag>(types, n));
}



// These are expected to be hana type constants.
auto get_type_indices = [](auto&& types) {
  return hana::transform(types, [](auto&& t) {
      return std::type_index(typeid(typename decltype(+t)::type));
    }
  );
};




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

  // Helper for generating n-ary function pointer types of the form:
  //   Ret f(Arg, Arg, Arg...)
  template <typename ...Args>
  using n_ary_fnptr_type = Ret(*)(Args...);


  // The dispatch table
  using key_type =
    typename decltype(hana::unpack(
      hana::replicate<hana::tuple_tag>(
        hana::type_c<std::type_index>,
        hana::size_c<Arity>
      ),
      hana::template_<std::tuple>
    ))::type;


  using value_type =
    typename decltype(hana::unpack(
      hana::replicate<hana::tuple_tag>(
        hana::type_c<Base&>,
        hana::size_c<Arity>
      ),
      hana::template_<n_ary_fnptr_type>
    ))::type;


  using map_type = std::map<key_type, value_type>;


  map_type _dispatch_table;

    

  // Helper for wrapping functions for insertion into map
  template <typename ...Args>
  struct wrap {
    static constexpr auto wrapped = [](always_base<Args>& ...args) {
      return Impl<Args...>{}(static_cast<Args&>(args)...);
    }; 
  };

    
public: 


  // XXX: There should really logically only be one needed
  multi_dispatcher(multi_dispatcher const&) = delete;
  multi_dispatcher(multi_dispatcher&&) = delete;
  multi_dispatcher& operator=(multi_dispatcher const&) = delete;
  multi_dispatcher& operator=(multi_dispatcher&&) = delete;


  multi_dispatcher() {
    // Setup dispatch table
    constexpr auto combos =
      detail::all_combos(hana::tuple_t<Ts...>, hana::size_c<Arity>);


    std::cout << hana::experimental::print(hana::type_c<map_type>) << '\n';
    std::cout << hana::experimental::print(combos) << '\n';
    

    hana::for_each(combos, [this](auto&& c) {
      using wrapper =
        typename decltype(hana::unpack(c, hana::template_<wrap>))::type;

      std::cout << hana::experimental::print(hana::type_c<decltype(wrapper::wrapped)>) << '\n';


      //this->_dispatch_table.insert(
      //  std::make_pair(
      //    hana::unpack(detail::get_type_indices(c),
      //      [](auto&& ...indices) {
      //        return key_type{std::forward<decltype(indices)>(indices)...};
      //      }
      //    ),
      //    // FIXME/TODO: what's the cleanest way to do this???
      //    wrapper::wrapped
      //  )
      //); 
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
