#include "../include/spleen.hpp"

Spleen::Spleen(unsigned pos_y, Display& controller, std::list<RBC>& rbc_pool, std::mutex& list_mutex)
    : Organ(pos_y, controller, rbc_pool, std::forward<std::mutex&>(list_mutex))
    , _life_thread(&Spleen::run, this)
{
}

Spleen::~Spleen()
{
    _life_thread.join();
}

void Spleen::destroy_rbc()
{
    std::lock_guard lg{ _list_mutex };
    for (auto it = _rbc_pool.begin(); it != _rbc_pool.end(); ++it) {
        if (it->get_y() == _y && it->get_x() >= 10 && it->get_x() <= 61) {
            if (it->get_dstate() == RBC_State::OLD || it->get_dstate() == RBC_State::DECAYED) {
                it->destroy();
                it = _rbc_pool.erase(it);
            }
        }
    }
}

void Spleen::run()
{
    int sleep_time;
    std::unique_lock lock{ _dp_controller.get_start_mutex() };
    _dp_controller.get_start_cv().wait(lock);
    lock.unlock();
    while (!_kill_switch) {
        for (int i = 0; i <= 5; ++i) {
            sleep_time = static_cast<int>(200 / _metabolism_speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
            nourish();
            //std::lock_guard lg{ _dp_controller.get_display_mutex() };
            _dp_controller.update_organ_state("SPLEEN", i * 20, _health, get_resources_state());
        }
        destroy_rbc();
        health_decay();
        inform_brain();
    }
}