#include "Renderer/RenderCommand.h"
#include "Shader/Shader.h"
#include "Resource/Material.h"
#include "Renderer/VertexArray.h"
#include <glad/glad.h>

namespace Avalon {

    std::unique_ptr<RendererAPI> RenderCommandAPI::s_RendererAPI = RendererAPI::Create();

    void MeshDrawCommand::Execute(uint32_t& activeShaderID, uint32_t& activeMaterialID) {
        if (!m_Shader || !m_VertexArray) return;

        // 1. Deduplicated Shader Binding
        uint32_t shaderID = m_Shader->GetShaderID();
        if (shaderID != activeShaderID) {
            m_Shader->Bind();
            activeShaderID = shaderID;
        }

        // 2. Deduplicated Material Binding
        if (m_Material) {
            uint32_t materialID = static_cast<uint32_t>(m_Material->GetSortKey() & 0xFFFF);
            if (materialID != activeMaterialID) {
                m_Material->Bind(m_Shader);
                activeMaterialID = materialID;
            }
        } else {
            if (activeMaterialID != 0) {
                // Unbind previous textures and UBO to clean state
                glBindTextureUnit(0, 0);
                glBindTextureUnit(1, 0);
                glBindTextureUnit(2, 0);
                glBindTextureUnit(3, 0);
                glBindBufferBase(GL_UNIFORM_BUFFER, 2, 0);
                activeMaterialID = 0;
            }
        }

        // 3. Set dynamic object transformations
        m_Shader->SetMat4("u_Model", m_Transform);
        m_Shader->SetMat3("u_NormalMatrix", m_NormalMatrix);

        // 4. Draw indexed geometry
        RenderCommandAPI::DrawIndexed(m_VertexArray);
    }

} // namespace Avalon
