#include <iostream>
#include <string>
#include <utility>

#include "multi.hpp"


struct object {
  std::string _name;

  object(std::string&& name) : _name(std::move(name)) {}
  object(std::string const& name) : _name(name) {}
  
  virtual void explode() = 0;
  virtual ~object() {}
};


struct shot : object {
  shot(std::string&& name) : object(name) {}
  shot(std::string const& name) : object(name) {}

  virtual void explode() override {
    std::cout << "shot " << _name << " dissipated\n";  
  }
};


struct ship : object {
  ship(std::string&& name) : object(name) {}
  ship(std::string const& name) : object(name) {}

  virtual void explode() override {
    std::cout << "ship " << _name
      << " became a fireball\n";  
  }
};


struct asteroid : object {
  asteroid(std::string&& name) : object(name) {}
  asteroid(std::string const& name) : object(name) {}

  virtual void explode() override {
    std::cout << "asteroid " << _name
      << " shattered into a million pieces\n";  
  }
};



template <typename ...Objs>
struct collide_impl {
  void operator()(Objs& ...) {
    std::cout << "Collision: generic\n";
  }
};


template <>
struct collide_impl<ship, asteroid> {
  void operator()(ship& s, asteroid& a) {
    std::cout << "Collision: Ship " << s._name << " ran into asteroid "
      << a._name << '\n';
  }
};


int main() {
  using namespace std::string_literals;


  ship our_hero{"Gradius"s};
  asteroid baddie{"Lumporocks"s};
  shot bullet{"Vulcan shot"s};


  object& hero_ref = our_hero;
  object& baddie_ref = baddie;
  object& bullet_ref = bullet;


  hero_ref.explode();
  baddie_ref.explode();
  bullet_ref.explode();


  multi::multi_dispatcher<
    object, 2, void, collide_impl, true,
    ship, asteroid, shot
    > collide;




  return 0;
}
