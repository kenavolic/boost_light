#include "lsg.h"
#include "lsm.h"

#include <functional>
#include <iostream>
#include <vector>

//--------------------------------------------------------
// Signal slot example
//--------------------------------------------------------

struct callback {
  void operator()() { std::cout << "callback called" << std::endl; }
};

struct callback2 {
  void operator()(int a, int b) {
    std::cout << "callback2 called with param " << a << " and " << b
              << std::endl;
  }
};

struct update_event {};
struct modified_event {};

struct image {
  lsg::signal<void(update_event)> sigup;
  lsg::signal<void(modified_event)> sigmod;

  void force_update() {
    sigup(update_event());
    sigmod(modified_event());
  }
};

struct viewer {
  image img;

  viewer() {
    img.sigup.connect([this](auto &&evt) {
      this->on_update(std::forward<decltype(evt)>(evt));
    });
    img.sigmod.connect([this](auto &&evt) {
      this->on_modified(std::forward<decltype(evt)>(evt));
    });
  }

  void on_update(update_event evt) {
    std::cout << "update evt receive" << std::endl;
  }

  void on_modified(modified_event) {
    std::cout << "modified evt receive" << std::endl;
  }
};

//--------------------------------------------------------
// State machine example
//--------------------------------------------------------

// static desc of the state machine
struct controller : lsm::state_machine_desc<controller> {
  // state
  struct state1 final : lsm::base_state {
    virtual void on_enter() override {
      std::cout << "enter state 1" << std::endl;
    }

    virtual void on_exit() override {
      std::cout << "exit state 1" << std::endl;
    }
  };

  struct state2 final : lsm::base_state {
    virtual void on_enter() override {
      std::cout << "enter state 2" << std::endl;
    }

    virtual void on_exit() override {
      std::cout << "exit state 2" << std::endl;
    }
  };

  struct state3 final : lsm::base_state {};

  // inputs
  struct input12 {};

  struct input13 {};

  struct input21 {
    int val{0};
  };

  // callbacks
  void on_input21(const input21 &i) {
    std::cout << "callback on input21 called" << std::endl;
    std::cout << "value = " << i.val << std::endl;
  }

  using me = controller;
  // define transition table
  using transition_table = lsm::transition_table_type<
      transition<state1, input12, state2>,
      transition_cb<state2, input21, state1, &me::on_input21>,
      transition<state1, input13, state3>>;

  using sm_type = lsm::state_machine_front<controller>;
  sm_type state_machine;

  controller() : state_machine{*this} {}
};

//--------------------------------------------------------
// Main
//--------------------------------------------------------

int main() {
  // Sig test
  std::cout << "------light signal test------\n" << std::endl;

  lsg::signal<void()> sig;
  lsg::signal<void(int, int)> sig2;

  sig.connect(callback());
  sig2.connect(callback2());
  sig();
  sig2(3, 4);
  viewer v;
  v.img.force_update();

  std::cout << std::endl;

  // State machine test
  std::cout << "------light state machine test------\n" << std::endl;
  controller ctrl;

  ctrl.state_machine.init<controller::state1>();
  ctrl.state_machine.transit(controller::input12{});
  ctrl.state_machine.transit(controller::input21{666});
  ctrl.state_machine.transit(controller::input13{});

  try {
    ctrl.state_machine.transit(controller::input21{666});
  } catch (const std::runtime_error &err) {
    std::cout << "Bad transition! It was expected!" << std::endl;
  }

  ctrl.state_machine.init<controller::state2>();
  ctrl.state_machine.transit(controller::input21{666});

  return 0;
}