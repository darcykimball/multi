#ifndef UTIL_HPP
#define UTIL_HPP


#include <typeinfo>
#include <typeindex>

#include <boost/hana.hpp>


namespace util {


template <typename T, typename>
using always_type = T;


namespace hana = boost::hana;


// Get all possible argument-type combinations
// XXX: All supplied types must be distinct
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


} // namespace util


#endif // UTIL_HPP
