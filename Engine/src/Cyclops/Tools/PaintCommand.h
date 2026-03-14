#pragma once

#include "Cyclops/Core/Command.h"
#include "Cyclops/Scene/Canvas.h"
#include "Cyclops/Renderer/Framebuffer.h"
#include "Cyclops/Core/Base.h"

namespace Cyclops
{
    class CYCLOPS_API PaintCommand : public Command
    {
    public:
        PaintCommand(Canvas* canvas, std::shared_ptr<Framebuffer> beforeState, int layerIndex, int frameIndex);

        virtual std::string GetName() const override;
        virtual void Execute() override;
        virtual void Undo() override;

    private:
        void Restore(std::shared_ptr<Framebuffer> snapshot);

        Canvas* m_Canvas;
        std::shared_ptr<Framebuffer> m_BeforeState;
        std::shared_ptr<Framebuffer> m_AfterState;
        int m_LayerIndex;
        int m_FrameIndex;
    };
}