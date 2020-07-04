/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta signal
 */

#include "Meta.hpp"

using namespace kF;

Meta::Signal::DelayedSlotMap Meta::Signal::_DelayedSlotMap {};

void Meta::Signal::ProcessDelayedSlots(void)
{
    auto holder = _DelayedSlotMap.find(std::this_thread::get_id());

    if (!holder)
        return;
    for (auto &slot : holder.value()) {
        if (slot.opaqueFunctor.unique())
            continue;
        else if (!slot.opaqueFunctor->invokeFunc(slot.opaqueFunctor->data, slot.receiver, slot.arguments.get()))
            throw std::runtime_error("Meta::Signal::ProcessDelayedSlots: Invalid slot signature");
    }
    holder.value().clear();
}