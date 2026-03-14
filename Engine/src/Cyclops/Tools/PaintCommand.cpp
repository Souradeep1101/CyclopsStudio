#include "Cyclops/Tools/PaintCommand.h" // Update path if needed

namespace Cyclops {

    PaintCommand::PaintCommand(Canvas* canvas, std::shared_ptr<Framebuffer> beforeState, int layerIndex, int frameIndex)
        : m_Canvas(canvas), m_BeforeState(beforeState), m_LayerIndex(layerIndex), m_FrameIndex(frameIndex)
    {
        // Capture "After" state immediately
        auto& layer = m_Canvas->GetLayers()[m_LayerIndex];
        auto currentBuffer = layer.GetActiveFramebuffer(m_FrameIndex);

        if (currentBuffer)
            m_AfterState = currentBuffer->Clone();
    }

    std::string PaintCommand::GetName() const
    {
        return "Paint Stroke";
    }

    void PaintCommand::Execute()
    {
        if (m_AfterState) Restore(m_AfterState);
    }

    void PaintCommand::Undo()
    {
        if (m_BeforeState) Restore(m_BeforeState);
    }

    void PaintCommand::Restore(std::shared_ptr<Framebuffer> snapshot)
    {
        auto& layer = m_Canvas->GetLayers()[m_LayerIndex];
        auto buffer = layer.GetActiveFramebuffer(m_FrameIndex);

        if (buffer)
        {
            buffer->BlitFrom(snapshot);
        }
    }

}