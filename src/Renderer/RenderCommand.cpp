#include "Renderer/RenderCommand.h"
#include "Shader/Shader.h"
#include "Resource/Material.h"
#include "Renderer/VertexArray.h"

namespace Avalon {

    std::unique_ptr<RendererAPI> RenderCommandAPI::s_RendererAPI = RendererAPI::Create();

    void MeshDrawCommand::Execute() {
        if (!m_Shader || !m_VertexArray) return;

        m_Shader->Bind();

        if (m_Material) {
            m_Material->Bind(m_Shader);
        }

        m_Shader->SetMat4("u_Model", m_Transform);
        m_Shader->SetMat3("u_NormalMatrix", m_NormalMatrix);

        RenderCommandAPI::DrawIndexed(m_VertexArray);
    }

} // namespace Avalon
