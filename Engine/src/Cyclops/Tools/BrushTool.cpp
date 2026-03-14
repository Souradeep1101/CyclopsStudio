#include "Cyclops/Tools/BrushTool.h"

// [CRITICAL] Wrap in namespace
namespace Cyclops {

    // --- BrushTool Implementation ---

    BrushTool::BrushTool(BrushEngine* engine)
        : m_Engine(engine)
    {
    }

    std::string BrushTool::GetName() const
    {
        return "Brush";
    }

    ToolType BrushTool::GetType() const
    {
        return ToolType::Brush;
    }

    bool BrushTool::OnPaint(const glm::vec2& pos, float pressure)
    {
        // Use the engine's interpolation logic
        m_Engine->Paint(pos, pressure);
        return true;
    }

    void BrushTool::OnEndStroke()
    {
        m_Engine->EndStroke();
    }

    // --- EraserTool Implementation ---

    EraserTool::EraserTool(BrushEngine* engine)
        : BrushTool(engine)
    {
    }

    ToolType EraserTool::GetType() const
    {
        return ToolType::Eraser;
    }

    std::string EraserTool::GetName() const
    {
        return "Eraser";
    }

    void EraserTool::ConfigureBlendState() const
    {
        // Eraser Math: Dest = Dest * (1 - SrcAlpha)
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA));
        GLCall(glBlendEquation(GL_FUNC_ADD));
    }

}