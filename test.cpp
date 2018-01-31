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


template <>
struct collide_impl<shot, asteroid> {
  void operator()(shot& s, asteroid& a) {
    std::cout << "Collision: Shot " << s._name << " destroyed asteroid "
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


  using dispatcher =
    multi::multi_dispatcher<
      object, 2, void, collide_impl, false,
      ship, asteroid, shot
      >;

  dispatcher collide;

  // XXX: Should fail with static assertion; wrong arity
  //collide(our_hero, our_hero, baddie);
  

  // XXX: Should fail with static assertion; type mismatch
  //auto huh = "huh"s;
  //collide(our_hero, huh);


  // Alright. Fingers crossed.
  collide(hero_ref, baddie_ref);
  collide(hero_ref, bullet_ref);
  collide(bullet_ref, baddie_ref);
  collide(hero_ref, hero_ref); // Doesn't make sense, but whatever



  return 0;
}
