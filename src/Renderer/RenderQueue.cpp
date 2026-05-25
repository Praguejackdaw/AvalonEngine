#include "Renderer/RenderQueue.h"
#include <algorithm>

namespace Avalon {

    void RenderQueue::Submit(std::unique_ptr<RenderCommand> command) {
        if (command) {
            m_Queue.push_back(std::move(command));
        }
    }

    void RenderQueue::Sort() {
        // Stable sort to maintain sub-ordering depth priority when state IDs are equal
        std::stable_sort(m_Queue.begin(), m_Queue.end(), 
            [](const std::unique_ptr<RenderCommand>& a, const std::unique_ptr<RenderCommand>& b) {
                return a->GetSortKey() < b->GetSortKey();
            }
        );
    }

    void RenderQueue::Execute() {
        m_DrawCallCount = 0;
        for (auto& cmd : m_Queue) {
            cmd->Execute();
            m_DrawCallCount++;
        }
    }

    void RenderQueue::Clear() {
        m_Queue.clear();
        m_DrawCallCount = 0;
    }

} // namespace Avalon
