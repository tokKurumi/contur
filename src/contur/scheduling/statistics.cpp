/// @file statistics.cpp
/// @brief Statistics implementation.

#include "contur/scheduling/statistics.h"

#include <algorithm>
#include <unordered_map>

namespace contur {

    struct Statistics::Impl
    {
        std::unordered_map<ProcessId, Tick> predictions;
        double alpha = 0.5;

        explicit Impl(double alpha)
            : alpha(std::clamp(alpha, 0.01, 1.0))
        {}
    };

    Statistics::Statistics(double alpha)
        : impl_(std::make_unique<Impl>(alpha))
    {}

    Statistics::~Statistics() = default;
    Statistics::Statistics(Statistics &&) noexcept = default;
    Statistics &Statistics::operator=(Statistics &&) noexcept = default;

    void Statistics::recordBurst(ProcessId pid, Tick burst)
    {
        auto it = impl_->predictions.find(pid);
        if (it == impl_->predictions.end())
        {
            impl_->predictions.emplace(pid, burst);
            return;
        }

        double oldValue = static_cast<double>(it->second);
        double newValue = static_cast<double>(burst);
        double predicted = impl_->alpha * newValue + (1.0 - impl_->alpha) * oldValue;
        it->second = static_cast<Tick>(predicted + 0.5);
    }

    Tick Statistics::predictedBurst(ProcessId pid) const noexcept
    {
        auto it = impl_->predictions.find(pid);
        if (it == impl_->predictions.end())
        {
            return 0;
        }
        return it->second;
    }

    bool Statistics::hasPrediction(ProcessId pid) const noexcept
    {
        return impl_->predictions.find(pid) != impl_->predictions.end();
    }

    void Statistics::clear(ProcessId pid)
    {
        impl_->predictions.erase(pid);
    }

    void Statistics::reset()
    {
        impl_->predictions.clear();
    }

    double Statistics::alpha() const noexcept
    {
        return impl_->alpha;
    }

} // namespace contur
