# Safeout

## Description

Some utility macros to safely exit a function in a generic way in order to:

* avoid resource leaks
* preserve objects states (invariants etc.)

## Use cases

### Resource management

* Regular resources management (nothing new here, macros here are just used to unify usage along use case).

```cpp
  void close_file(std::FILE* fp)
  {
      std::fclose(fp);
  }

  std::FILE* open_file() {
    // _safeout_push just use unique_ptr as you would use it directly
    _safeout_push(std::fopen("/tmp/demo.txt", "r"), &close_file);

    // retrieve the ressource if we have to manipulate it
    [[maybe_unused]] auto* file = _safeout_get();

    // code that can trigger error

    //...
    if (/*error1*/) {
      //close_file(file); -> no need
      return nullptr;
    }

    //...
    if (/*error2*/) {
      //close_file(file); -> no need
      return nullptr;
    }

    //...
    sublib_that_can_throw();

    _safeout_pop(); // release the safety
    return file;
  }

  void local_cpp_resources() {
    _safeout_push_0(new int);
    _safeout_push_1(new int[5]);

    // can manipulate the allocated ressources
    auto* number = _safeout_get_0();
    auto* array = _safeout_get_1();

    *number = 5;
    array[0] = 3;
  }
```

### State coherency

* Strong guarantees

```cpp
  /// Fake engine class
  /// \invariant tank empty -> speed = 0; tank not empty -> speed >= 0
  class Engine
  {
  private:
    unsigned _fuel_level{};
    unsigned _speed{};

  public:
    
    //...

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

    // ...
  };

  int main() {
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
```

### Generic error management

* Actions to be performed on error

```cpp
  void on_check_engine(Engine& engine)
  {

    _safeout_push([&engine]()
    {
      engine.stop();
    });

    // ...

    if (error1)
    {
      engine.stop();
      return;
    }

    // ...

    if (error2)
    {
      engine.stop();
      return;
    }

    void sublib_that_can_throw();

    _safeout_pop();
  }
```

## Compile & run example

```shell
> mkdir build && cd build
> g++ -o ./safeout ../main.cpp -I.. -std=c++17
> ./safeout
```
