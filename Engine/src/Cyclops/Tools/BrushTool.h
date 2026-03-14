#pragma once

#include "Cyclops/Tools/Tool.h"
#include "Cyclops/Tools/BrushEngine.h"
#include "Cyclops/Platform/OpenGL/OpenGLDebug.h"
#include "Cyclops/Core/Base.h"
#include <glad/glad.h>

namespace Cyclops
{
    class CYCLOPS_API BrushTool : public Tool
    {
    public:
        BrushTool(BrushEngine* engine); // Declaration only

        virtual std::string GetName() const override;
        virtual ToolType GetType() const override;
        virtual bool OnPaint(const glm::vec2& pos, float pressure) override;
        virtual void OnEndStroke() override;

    protected:
        BrushEngine* m_Engine;
    };

    class CYCLOPS_API EraserTool : public BrushTool
    {
    public:
        EraserTool(BrushEngine* engine);

        virtual ToolType GetType() const override;
        virtual std::string GetName() const override;
        virtual void ConfigureBlendState() const override;
    };
}