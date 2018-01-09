#ifndef MULTI_HPP
#define MULTI_HPP


#include <type_traits>

#include <boost/hana.hpp>


namespace detail {

  // Here we go
  namespace hana = boost::hana;


  // Get all possible argument-type combinations
  // TODO: use some sort of canonical ordering, etc.
  constexpr auto all_combos(size_t N, auto&& types) {
    return types
      | [&](auto&& t) { 
          return hana::plus(
            lift<hana::tuple_tag>(std::forward<decltype(t)>(t)),
            all_combos(N - 1, std::forward<decltype(types)>types));
        };
  };

  
  // Get the address of a function template instantiation
  // FIXME: Make non-function templates a hard error; use is_valid/declval?
  template <template <typename ...> FnTmpl, typename ...Ts>
  constexpr auto get_instantiation_impl = &FnTmpl<Ts...>;


  template <template <typename ...> FnTmpl>
  constexpr auto get_instantiation = [](auto&& types) {
    decltype(types)
  };
  
} // namespace detail


namespace multi {

  
  namespace hana = boost::hana;


// The multimethod dispatcher functor (c++) 
// TODO: specialization for when CTTI (https://github.com/Manu343726/ctti)
// is available
template <
  typename Base = void,
  size_t Arity,
  template <typename ...> Impl,
  typename ...Ts
  > 
class multi_dispatcher {

  // Helper type-level const
  template <size_t> 
  using always_base = Base;


  // Dispatch table
  static constexpr auto make_dispatch_table() {
    auto combos = all_combos(Arity, hana::tuple_t<Ts...>);
    auto key_values = hana::transform(combos, [](auto&& c) {
      hana::make_pair(c, get_instantiation(std::forward<decltype(c)>);
    });


    return hana::unpack(key_values, hana::make_map);
  }
          

    
public:


  template <typename ...Unused>
  void operator()(always_base<Unused>&...) {
    // XXX: Hopefully this is the sanest place to induce an error
    static_assert(sizeof...(Unused) == Arity,
      "Arity mismatch for multimethod call");

    
    // TODO: Get runtime types of arguments
   
    
    // TODO: Lookup combo in table
    

    // TODO: static_cast arguments to right types
  }
};


} // namespace multi



#endif // MULTI_HPP
