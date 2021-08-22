#include "emu_core/simulation/simulation.hpp"
#include <boost/scope_exit.hpp>
#include <chrono>

namespace emu {

EmuSimulation::Result EmuSimulation::Run(std::chrono::nanoseconds timeout) {
    auto start = std::chrono::steady_clock::now();

    Result result;
    try {
        BOOST_SCOPE_EXIT_ALL(&) {
            auto end = std::chrono::steady_clock::now();
            auto delta =
                std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            result.duration = static_cast<double>(delta.count()) / 1.0e6;
            result.cpu_cycles = clock->CurrentCycle();
        };

        clock->Reset();

        if (timeout.count() > 0) {
            cpu->ExecuteFor(timeout);
        } else {
            cpu->Execute();
        }
    } catch (const emu6502::cpu::ExecutionHalted &e) {
        result.halt_code = e.halt_code;
    } catch (const std::exception &e) {
        throw SimulationFailedException(fmt::format("{}: {}", typeid(e).name(), e.what()),
                                        std::current_exception(), result);
    }

    return result;
}

} // namespace emu
