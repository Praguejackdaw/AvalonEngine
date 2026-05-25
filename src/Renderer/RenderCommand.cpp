#include "Renderer/RenderCommand.h"

namespace Avalon {

    std::unique_ptr<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

} // namespace Avalon
