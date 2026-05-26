#include "Renderer/RenderQueue.h"
#include <algorithm>

namespace Avalon {

    void RenderQueue::Submit(std::unique_ptr<RenderCommand> command) {
        if (command) {
            uint64_t key = command->GetSortKey();
            uint32_t index = static_cast<uint32_t>(m_Queue.size());

            m_Queue.push_back(std::move(command));
            m_SortQueue.push_back({ key, index });
        }
    }

    void RenderQueue::Sort() {
        // Flat contiguous sort to maximize CPU Cache line efficiency and eliminate virtual lookups
        std::sort(m_SortQueue.begin(), m_SortQueue.end(), 
            [](const RenderElement& a, const RenderElement& b) {
                return a.SortKey < b.SortKey;
            }
        );
    }

    void RenderQueue::Execute() {
        m_DrawCallCount = 0;
        uint32_t activeShaderID = 0;
        uint32_t activeMaterialID = 0;

        for (const auto& element : m_SortQueue) {
            auto& cmd = m_Queue[element.CommandIndex];
            if (cmd) {
                cmd->Execute(activeShaderID, activeMaterialID);
                m_DrawCallCount++;
            }
        }
    }

    void RenderQueue::Clear() {
        m_Queue.clear();
        m_SortQueue.clear();
        m_DrawCallCount = 0;
    }

} // namespace Avalon
