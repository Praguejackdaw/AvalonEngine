#pragma once

#include "Renderer/RenderCommand.h"
#include <vector>
#include <memory>

namespace Avalon {

    struct RenderElement {
        uint64_t SortKey;
        uint32_t CommandIndex;
    };

    class RenderQueue {
    public:
        RenderQueue() = default;
        ~RenderQueue() = default;

        // Submits a polymorphic RenderCommand packet
        void Submit(std::unique_ptr<RenderCommand> command);

        // Sorts commands to minimize shader/material state switches
        void Sort();

        // Executes all queued commands on the GPU in sequence
        void Execute();

        // Flushes the queue
        void Clear();

        uint32_t GetDrawCallCount() const { return m_DrawCallCount; }

    private:
        std::vector<std::unique_ptr<RenderCommand>> m_Queue;
        std::vector<RenderElement> m_SortQueue;
        uint32_t m_DrawCallCount = 0;
    };

} // namespace Avalon
