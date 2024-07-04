#include "lpo.h"
#include "lsm.h"

#include <functional>
#include <iostream>
#include <vector>

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
// Program option example
//--------------------------------------------------------

struct my_opts {
  bool f1;
  bool f2;
  int i;
  double d;
  std::string str;
  std::string first_pos;
  int int_pos;
  bool b;
  std::vector<std::string> vstr;
};

void parse_options(int argc, char **argv) {
  // flags: --flag1 -f1 / --flag2 -f2
  // opts: -intopt -i integer in [0..10] / -dopt_longname double / -sopt -s
  // string (mandatory) positional opts: STR_POS (str) and INT_POS(int)

  // List all possible type in your options in the prog options declaration
  lpo::program_options<int, double, std::string, bool, std::vector<std::string>>
      po{argv[0], "Program help:"};
  my_opts opts;

  po.add_flag({"flag1", "f1", "flag 1", &(opts.f1)})
      .add_flag({"flag2", "f2", "flag 2", &(opts.f2)})
      .add_opt<int>({"intopt", "i", "ranged int option", &(opts.i), 0, 0, 10},
                    true)
      .add_opt<double>({"dopt_long-name", "", "double option", &(opts.d), 0.0f})
      .add_opt<std::vector<std::string>>(
          {"vecopt", "v", "vec option", &(opts.vstr)})
      .add_opt<bool>({"bopt", "b", "bool option", &(opts.b), true})
      .add_opt<std::string>(
          {"sopt", "s", "string option", &(opts.str), "dummy"})
      .add_pos_opt<std::string>({"string first pos option", &(opts.first_pos)})
      .add_pos_opt<int>({"int second pos option", &(opts.int_pos), 1, 3});

  po.parse(argc, argv);

  std::cout << "options values" << std::endl;
  std::cout << "opts.f1 = " << opts.f1 << std::endl;
  std::cout << "opts.f2 = " << opts.f2 << std::endl;
  std::cout << "opts.intopt = " << opts.i << std::endl;
  std::cout << "opts.dopt_long-name = " << opts.d << std::endl;
  std::cout << "opts.sopt = " << opts.str << std::endl;
  std::cout << "opts.bopt = " << opts.b << std::endl;
  std::cout << "opts.STR_POS = " << opts.first_pos << std::endl;
  std::cout << "opts.INT_POS = " << opts.int_pos << std::endl;

  for (const auto &o : opts.vstr) {
    std::cout << "opts.vstr = " << o << std::endl;
  }

  lpo::program_options<int, double, std::string> nopos{"testnopos",
                                                       "Program help:"};

  nopos.add_flag({"flag1", "f1", "flag 1", &(opts.f1)})
      .add_flag({"flag2", "f2", "flag 2", &(opts.f2)})
      .add_opt<int>({"intopt", "i", "ranged int option", &(opts.i), 0, 0, 10})
      .add_opt<double>({"dopt", "d", "double option", &(opts.d), 0.0f})
      .add_opt<std::string>(
          {"sopt", "s", "string option", &(opts.str), "dummy"});

  std::cout << nopos;

  lpo::program_options<int, double, std::string> noopt{"testnoopt",
                                                       "Program help:"};
  std::cout << noopt;

  // some error test
  try {
    noopt.add_flag({"", "f1", "flag 1", &(opts.f1)});
  } catch (const std::runtime_error &err) {
    std::cout << "caught expected error " << err.what() << std::endl;
  }

  try {
    noopt.add_flag({"add&{", "f1", "flag 1", &(opts.f1)});
  } catch (const std::runtime_error &err) {
    std::cout << "caught expected error " << err.what() << std::endl;
  }

  try {
    noopt.add_flag({"flag1", "f&", "flag 1", &(opts.f1)});
  } catch (const std::runtime_error &err) {
    std::cout << "caught expected error " << err.what() << std::endl;
  }

  try {
    noopt.add_opt<int>({"", "i", "ranged int option", &(opts.i), 0, 0, 10});
  } catch (const std::runtime_error &err) {
    std::cout << "caught expected error " << err.what() << std::endl;
  }

  try {
    noopt.add_opt<int>(
        {"int test", "i", "ranged int option", &(opts.i), 0, 0, 10});
  } catch (const std::runtime_error &err) {
    std::cout << "caught expected error " << err.what() << std::endl;
  }

  try {
    noopt.add_opt<int>(
        {"intopt", "i a", "ranged int option", &(opts.i), 0, 0, 10});
  } catch (const std::runtime_error &err) {
    std::cout << "caught expected error " << err.what() << std::endl;
  }

  try {
    noopt.add_opt<int>(
        {"intopt", "i", "ranged int option", &(opts.i), 0, 0, 10});
    noopt.add_opt<int>(
        {"intopt", "i2", "ranged int option", &(opts.i), 0, 0, 10});
  } catch (const std::runtime_error &err) {
    std::cout << "caught expected error " << err.what() << std::endl;
  }

  try {
    noopt.add_opt<int>(
        {"intopt", "i", "ranged int option", &(opts.i), 0, 0, 10});
    noopt.add_opt<int>(
        {"intopt2", "i", "ranged int option", &(opts.i), 0, 0, 10});
  } catch (const std::runtime_error &err) {
    std::cout << "caught expected error " << err.what() << std::endl;
  }

  std::cout << "\n\n";
}

//--------------------------------------------------------
// Main
//--------------------------------------------------------

// NOTE: call it with following args to test lpo
// ./demo --flag2 -f1 --dopt_long-name 4.0 --intopt 10 --sopt "toto" pos1 3
int main(int argc, char **argv) {

  // Program options test
  std::cout << "------light program options test------\n" << std::endl;
  parse_options(argc, argv);

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