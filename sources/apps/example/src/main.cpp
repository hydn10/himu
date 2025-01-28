#include <stdexec/execution.hpp>

#include <iostream>
#include <map>
#include <memory>
#include <print>
#include <vector>


namespace ex = stdexec;


class scheduler;

class simulation
{
public:
  struct operation_base
  {
    virtual ~operation_base() = default;
    virtual void
    notify() = 0;
  };

  template<typename Receiver>
  class operation : public operation_base
  {
    Receiver receiver_;

  public:
    operation(Receiver receiver)
        : receiver_(std::move(receiver))
    {
    }

    void
    notify() override
    {
      ex::set_value(std::move(receiver_));
    }
  };

  int
  current_step() const
  {
    return current_step_;
  }

  void
  step()
  {
    current_step_++;
    auto it = scheduled_.find(current_step_);
    if (it != scheduled_.end())
    {
      for (auto &op : it->second)
      {
        op->notify();
      }
      scheduled_.erase(it);
    }
  }

  scheduler
  get_scheduler(int steps);

  void
  insert(int step, std::unique_ptr<operation_base> op)
  {
    scheduled_[step].push_back(std::move(op));
  }

private:
  int current_step_ = 0;
  std::map<int, std::vector<std::unique_ptr<operation_base>>> scheduled_;
};


class after_steps_sender;

class scheduler
{
  simulation *simulation_;
  int steps_;

public:
  scheduler(simulation &simulation, int steps)
      : simulation_{&simulation}
      , steps_{steps}
  {
  }

  class schedule_env
  {
  public:
    simulation *simulation;
    int steps;

  private:
    friend auto
    tag_invoke(stdexec::get_completion_scheduler_t<stdexec::set_value_t>, schedule_env const &env) noexcept -> scheduler
    {
      return scheduler{*env.simulation, env.steps};
    }
  };

  friend auto
  operator==(scheduler const &lhs, scheduler const &rhs) -> bool = default;

  after_steps_sender
  schedule();
};


scheduler
simulation::get_scheduler(int steps)
{
  return {*this, steps};
}


class after_steps_sender
{
  using completion_sigs = stdexec::completion_signatures<stdexec::set_value_t()>;

  scheduler::schedule_env env_;

public:
  using sender_concept = stdexec::sender_t;
  using sigs_removeme = completion_sigs;

  auto
  get_env() const noexcept -> scheduler::schedule_env
  {
    return env_;
  }

  auto
  get_completion_signatures(stdexec::__ignore = {}) const noexcept -> completion_sigs
  {
    return {};
  }

  after_steps_sender(scheduler::schedule_env env)
      : env_{env}
  {
  }

  template<typename Receiver>
  struct operation_state
  {
    simulation &simulation_;
    int steps_;
    Receiver receiver_;

    operation_state(simulation &simulation, int steps, Receiver receiver)
        : simulation_(simulation)
        , steps_(steps)
        , receiver_(std::move(receiver))
    {
    }

    void
    start() noexcept
    {
      int const target_step = simulation_.current_step() + steps_;
      simulation_.insert(target_step, std::make_unique<simulation::operation<Receiver>>(std::move(receiver_)));
    }
  };

  template<stdexec::receiver_of<completion_sigs> Receiver>
  operation_state<Receiver>
  connect(Receiver receiver) const &
  {
    /*return stdexec::__t<__schedule_operation<stdexec::__id<Receiver>>>(
        std::in_place, *__env_.__context_, static_cast<Receiver &&>(__receiver));*/
    return {*env_.simulation, env_.steps, std::move(receiver)};
  }
};


after_steps_sender
scheduler::schedule()
{
  return {schedule_env{simulation_, steps_}};
}


struct scopity
{
  struct env
  {
  };

  struct cout_receiver
  {
    using receiver_concept = ex::receiver_t;

    void
    set_value() noexcept
    {
      std::println("log -> set_value");
    }

    void
    set_error(auto &&err) noexcept
    {
      std::println("log -> set_error");
      std::terminate();
    }

    void
    set_stopped() noexcept
    {
      std::println("log -> set_stopped");
      std::terminate();
    }

    auto
    get_env() const noexcept -> env
    {
      return {};
    }
  };
};


int
main()
{
  simulation sim;

  ex::scheduler auto sched = sim.get_scheduler(3);
  ex::sender auto sdr = ex::schedule(sched);

  ex::sender auto agent = sdr | ex::then([] { std::println("hello after 3 steps!"); });

  ex::operation_state auto op = sdr.connect(scopity::cout_receiver{});
  op.start();

  // No idea why this doesnt work.
  // More documetation needed on how to implement receivers i guess.
  // auto op = ex::connect(std::move(agent), scopity::cout_receiver{});


  for (int i = 0; i < 10; ++i)
  {
    std::println("Simulation step: {}", sim.current_step());
    sim.step();
  }
}


constexpr void
asserts()
{
  // is scheduler
  static_assert(ex::__has_schedule<scheduler>);
  static_assert(ex::__sender_has_completion_scheduler<scheduler>);
  static_assert(ex::equality_comparable<ex::__decay_t<scheduler>>);
  static_assert(ex::copy_constructible<ex::__decay_t<scheduler>>);

  // is sender
  static_assert(ex::enable_sender<ex::__decay_t<after_steps_sender>>);
  static_assert(ex::environment_provider<ex::__cref_t<after_steps_sender>>);
  static_assert(ex::__detail::__consistent_completion_domains<after_steps_sender>);
  static_assert(ex::move_constructible<ex::__decay_t<after_steps_sender>>);
  static_assert(ex::constructible_from<ex::__decay_t<after_steps_sender>, after_steps_sender>);

  // is receiver
  static_assert(ex::receiver_of<scopity::cout_receiver, after_steps_sender::sigs_removeme>);

  // is operation state
  static_assert(ex::operation_state<after_steps_sender::operation_state<scopity::cout_receiver>>);
}
