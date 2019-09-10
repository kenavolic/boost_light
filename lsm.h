// Copyright 2019 Ken Avolic <kenavolic@none.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <stdexcept>
#include <type_traits>
#include <variant>

///
/// Light state machine (lsm) is a simple template
/// to build a state machine engine customized for your
/// needs. Base implementation is taken from mvm that follows
/// the same structure as boost msm.
///

//--------------------------------------------------------
// Internal utilities
//--------------------------------------------------------

namespace lsm::utilities {
// fallback type
struct nonsuch {
  nonsuch() = delete;
  ~nonsuch() = delete;
  nonsuch(const nonsuch &) = delete;
  void operator=(const nonsuch &) = delete;
};
} // namespace lsm::utilities

//--------------------------------------------------------
// List utilies
//--------------------------------------------------------

namespace lsm::list {
// generic type list algorithms

template <typename List> struct is_empty;

template <template <typename...> typename List, typename... Items>
struct is_empty<List<Items...>> : std::false_type {};

template <template <typename...> typename List>
struct is_empty<List<>> : std::true_type {};

template <typename List> using is_empty_t = typename is_empty<List>::type;

template <typename List> constexpr bool is_empty_v = is_empty_t<List>::value;

template <typename List> struct size;

template <template <typename...> typename List, typename... Items>
struct size<List<Items...>> {
  static constexpr std::size_t value = sizeof...(Items);
};

template <template <typename...> typename List> struct size<List<>> {
  static constexpr std::size_t value = 0;
};

template <typename List> constexpr auto size_v = size<List>::value;

template <typename List> struct front;

template <template <typename...> typename List, typename Head, typename... Tail>
struct front<List<Head, Tail...>> {
  using type = Head;
};

template <typename List> using front_t = typename front<List>::type;

template <typename List> struct robust_front { using type = front_t<List>; };

template <template <typename...> typename List> struct robust_front<List<>> {
  using type = utilities::nonsuch;
};

template <typename List>
using robust_front_t = typename robust_front<List>::type;

template <typename List> struct pop_front;

template <template <typename...> typename List, typename Head, typename... Tail>
struct pop_front<List<Head, Tail...>> {
  using type = List<Tail...>;
};

template <template <typename...> typename List> struct pop_front<List<>> {
  using type = List<>;
};

template <typename List> using pop_front_t = typename pop_front<List>::type;

template <typename Item, typename List> struct push_front;

template <typename Item, template <typename...> typename List,
          typename... Items>
struct push_front<Item, List<Items...>> {
  using type = List<Item, Items...>;
};

template <typename Item, typename List>
using push_front_t = typename push_front<Item, List>::type;

template <typename List> struct pop_back;

template <template <typename...> typename List, typename Head, typename... Tail>
struct pop_back<List<Head, Tail...>> {
  using type = push_front_t<Head, typename pop_back<List<Tail...>>::type>;
};

template <template <typename...> typename List, typename Head>
struct pop_back<List<Head>> {
  using type = List<>;
};

template <typename List> using pop_back_t = typename pop_back<List>::type;

namespace details {
template <typename Item, typename List, bool = is_empty_v<List>>
struct push_back_impl;

template <typename Item, typename List>
struct push_back_impl<Item, List, false> {
  using type =
      push_front_t<front_t<List>,
                   typename push_back_impl<Item, pop_front_t<List>>::type>;
};

template <typename Item, typename List>
struct push_back_impl<Item, List, true> {
  using type = push_front_t<Item, List>;
};
} // namespace details

template <typename Item, typename List>
struct push_back : details::push_back_impl<Item, List> {};

template <typename Item, typename List>
using push_back_t = typename push_back<Item, List>::type;

template <typename Item, typename List, bool = is_empty_v<List>> struct has {
  static constexpr bool value =
      !is_empty_v<List> && (std::is_same<Item, robust_front_t<List>>::value ||
                            has<Item, pop_front_t<List>>::value);
};

template <typename Item, typename List> struct has<Item, List, false> {
  static constexpr bool value = std::is_same_v<Item, front_t<List>> ||
                                has<Item, pop_front_t<List>>::value;
};

template <typename Item, typename List> struct has<Item, List, true> {
  static constexpr bool value = false;
};

template <typename Item, typename List>
constexpr bool has_v = has<Item, List>::value;

template <typename List, bool = (size_v<List>> 1)> struct remove_dup;

template <typename List> struct remove_dup<List, true> {
  using Head = front_t<List>;
  using Tail = pop_front_t<List>;
  using type =
      std::conditional_t<has_v<Head, Tail>, typename remove_dup<Tail>::type,
                         push_front_t<Head, typename remove_dup<Tail>::type>>;
};

template <typename List> struct remove_dup<List, false> { using type = List; };

template <typename List> using remove_dup_t = typename remove_dup<List>::type;

template <typename List1, typename List2, bool = is_empty_v<List2>>
struct concat;

template <typename List1, typename List2> struct concat<List1, List2, false> {
  using type = push_front_t<front_t<List2>,
                            typename concat<List1, pop_front_t<List2>>::type>;
};

template <typename List1, typename List2> struct concat<List1, List2, true> {
  using type = List1;
};

template <typename List1, typename List2>
using concat_t = typename concat<List1, List2>::type;

template <typename List1, typename List2>
using merge_t = remove_dup_t<concat_t<List1, List2>>;

template <typename... Lists> struct concat_all;

template <typename L1, typename L2, typename... Ls>
struct concat_all<L1, L2, Ls...> {
  using type = concat_t<L1, typename concat_all<L2, Ls...>::type>;
};

template <typename L> struct concat_all<L> { using type = L; };

template <typename... Lists>
using concat_all_t = typename concat_all<Lists...>::type;

template <template <typename...> typename TList, typename SList> struct rebind;

template <template <typename...> typename TList,
          template <typename...> typename SList, typename... Args>
struct rebind<TList, SList<Args...>> {
  using type = TList<Args...>;
};

template <template <typename...> typename TList, typename SList>
using rebind_t = typename rebind<TList, SList>::type;

template <typename... Ts> struct mplist {};
} // namespace lsm::list

//--------------------------------------------------------
// Internal details
//--------------------------------------------------------

namespace lsm::details {
// state type aggregator
template <typename List, bool = lsm::list::is_empty_v<List>>
struct set_state_types_aggregator;

template <typename List> struct set_state_types_aggregator<List, false> {
  using head = lsm::list::front_t<List>;
  using type = lsm::list::merge_t<
      lsm::list::mplist<typename head::source_state_type,
                        typename head::target_state_type>,
      typename set_state_types_aggregator<lsm::list::pop_front_t<List>>::type>;
};

template <typename List> struct set_state_types_aggregator<List, true> {
  using type = lsm::list::mplist<>;
};

template <typename List>
using set_state_types_aggregator_t =
    typename set_state_types_aggregator<List>::type;

// transition finder
template <typename State, typename Input, typename List,
          bool = lsm::list::is_empty_v<List>>
struct tx_finder;

template <typename State, typename Input, typename List>
struct tx_finder<State, Input, List, false> {
  using current_tx = lsm::list::front_t<List>;
  using type = std::conditional_t<
      std::is_same_v<State, typename current_tx::source_state_type> &&
          std::is_same_v<Input, typename current_tx::input_type>,
      current_tx,
      typename tx_finder<State, Input, lsm::list::pop_front_t<List>>::type>;
};

template <typename State, typename Input, typename List>
struct tx_finder<State, Input, List, true> {
  using type = utilities::nonsuch;
};

template <typename State, typename Input, typename List>
using tx_finder_t = typename tx_finder<State, Input, List>::type;
} // namespace lsm::details

//--------------------------------------------------------
// State machine traits
//--------------------------------------------------------

namespace lsm::traits {
template <typename T> struct state_machine_traits {
  using transition_table_type = typename T::transition_table;
  using state_list_type = list::rebind_t<
      std::variant,
      details::set_state_types_aggregator_t<transition_table_type>>;
};
} // namespace lsm::traits

//--------------------------------------------------------
// State machine engine implementation
//--------------------------------------------------------

namespace lsm {
///
///@brief Type used to describe transitions
///
template <typename... Ts> using transition_table_type = list::mplist<Ts...>;

///
/// @brief Base structure used for state definition
///
struct base_state {
  virtual void on_enter() {
    // no op
  }

  virtual void on_exit() {
    // no op
  }
};

///
/// @brief Base state machine descriptor
///
template <typename Base> class state_machine_desc {
private:
  template <typename SourceState, typename Input, typename TargetState>
  struct base_transition {
    using source_state_type = SourceState;
    using input_type = Input;
    using target_state_type = TargetState;
  };

public:
  template <typename SourceState, typename Input, typename TargetState,
            auto Func>
  struct transition_cb : base_transition<SourceState, Input, TargetState> {

    template <typename SM, typename... Args>
    static auto apply(SM &sm, Args &&... args) {
      return (sm.*Func)(std::forward<Args>(args)...);
    }
  };

  template <typename SourceState, typename Input, typename TargetState>
  struct transition : base_transition<SourceState, Input, TargetState> {

    template <typename VM, typename Arg> static void apply(VM &vm, Arg arg) {
      // sink
    }
  };

  // To be continued... Create new transition type here suited to your needs
};

///
/// @brief Base state machine frontend
///
template <typename T> class state_machine_front {
  using table_type =
      typename traits::state_machine_traits<T>::transition_table_type;
  using state_list_type =
      typename traits::state_machine_traits<T>::state_list_type;

private:
  T &m_sm;
  state_list_type m_current_state;

  template <typename State, typename Input>
  void apply_transition(const Input &input) {

    using tx = details::tx_finder_t<State, Input, table_type>;

    if constexpr (!std::is_same_v<tx, utilities::nonsuch>) {
      using next_state = typename tx::target_state_type;

      // exit state
      std::get<State>(m_current_state).on_exit();

      // enter new state
      m_current_state = next_state();
      std::get<next_state>(m_current_state).on_enter();

      // apply transition
      tx::apply(m_sm, input);
    } else {
      // For your needs, you could add custom error handler to this class
      // easily
      throw std::runtime_error("bad transition");
    }
  }

public:
  state_machine_front(T &sm) : m_sm{sm} {}

  template <typename State> void init() {
    m_current_state = State();
    std::get<State>(m_current_state).on_enter();
  }

  template <typename Input> void transit(const Input &input) {
    std::visit(
        [this, &input](auto &&arg) {
          using S = std::decay_t<decltype(arg)>;
          this->apply_transition<S, Input>(input);
        },
        m_current_state);
  }
};
} // namespace lsm