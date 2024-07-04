#include "safeout.hpp"

#include <iostream>

namespace
{

  void close_file(std::FILE* fp)
  {
    std::fclose(fp);
  }

  std::FILE* open_file()
  {
    // _safeout_push just use unique_ptr as you would use it directly
    _safeout_push(std::fopen("/tmp/demo.txt", "r"), &close_file);

    // retrieve the ressource if we have to manipulate it
    [[maybe_unused]] auto* file = _safeout_get();

    // code that can trigger error
    // if (...) {
    //   //close_file(file);
    //   return nullptr;
    // }

    // if (...) {
    //   //close_file(file);
    //   return nullptr;
    // }

    //
    //void sublib_that_can_throw();

    _safeout_pop();
    return file;
  }

  void local_cpp_resources()
  {
    _safeout_push_0(new int);
    _safeout_push_1(new int[5]);

    // can manipulate the allocated ressources
    auto* number = _safeout_get_0();
    auto* array = _safeout_get_1();

    *number = 5;
    array[0] = 3;
  }

  /// Fake engine class
  /// \invariant tank empty -> speed = 0; tank not empty -> speed >= 0
  class Engine
  {
  private:
    unsigned _fuel_level{};
    unsigned _speed{};

  public:
    void fill_tank()
    {
      _fuel_level = 3;
    }
    void start() {}
    void stop() {}

    void speed_up()
    {
      _speed++;
    }

    void upate_state_from_sensors(unsigned(*fuel_cb)(), unsigned(*speed_cb)())
    {
      // bad way of handling it: invariant can be broken
      //_fuel_level = fuel_cb();
      //_speed = speed_cb();

      // better way: separe code that can throw from code that modifies invariant
      //auto fuel_level = fuel_cb();
      //auto speed = speed_cb();
      //_fuel_level = fuel_level;
      //_speed = speed_cb();

      _safeout_push([this, tmp = *this]()
      {
        *this = tmp;
      });

      _fuel_level = fuel_cb();
      _speed = speed_cb();

      _safeout_pop();
    }

    unsigned fuel_level() const
    {
      return _fuel_level;
    }
    unsigned speed() const
    {
      return _speed;
    }
  };
}

int main(int, char**)
{
  std::cout << "Safeout examples\n";
  //open_file();
  local_cpp_resources();

  // Test the engine
  Engine engine;
  engine.fill_tank();
  engine.speed_up();
  std::cout << "engine state: " << engine.fuel_level() << "L/" << engine.speed() << "km/h\n";

  engine.upate_state_from_sensors([]() {return 2U; }, []() {return 10U; });
  std::cout << "engine state: " << engine.fuel_level() << "L/" << engine.speed() << "km/h\n";

  try
  {
    engine.upate_state_from_sensors([]() { return 0U; }, []() { throw 666; return 0U; });
  }
  catch (...) {}

  std::cout << "engine state: " << engine.fuel_level() << "L/" << engine.speed() << "km/h\n";

}