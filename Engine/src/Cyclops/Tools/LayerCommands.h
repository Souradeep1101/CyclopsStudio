#pragma once

#include "Cyclops/Core/Command.h"
#include "Cyclops/Scene/Canvas.h"
#include "Cyclops/Core/Base.h"

// [CRITICAL] Needed for backup storage
#include "Cyclops/Renderer/Framebuffer.h" 

namespace Cyclops {

    // =============================================================
    // LAYER COMMANDS
    // =============================================================

    class CYCLOPS_API AddLayerCommand : public Command
    {
    public:
        AddLayerCommand(Canvas* canvas);
        virtual std::string GetName() const override;
        virtual void Execute() override;
        virtual void Undo() override;
    private:
        Canvas* m_Canvas;
        int m_AddedIndex = 0;
    };

    class CYCLOPS_API RemoveLayerCommand : public Command
    {
    public:
        RemoveLayerCommand(Canvas* canvas, int index);
        virtual std::string GetName() const override;
        virtual void Execute() override;
        virtual void Undo() override;
    private:
        Canvas* m_Canvas;
        int m_Index;
        CanvasLayer m_BackupLayer; // Stores entire layer state
    };

    class CYCLOPS_API MoveLayerCommand : public Command
    {
    public:
        MoveLayerCommand(Canvas* canvas, int fromIndex, int toIndex);
        virtual std::string GetName() const override;
        virtual void Execute() override;
        virtual void Undo() override;
    private:
        Canvas* m_Canvas;
        int m_From, m_To;
    };

    // =============================================================
    // KEYFRAME COMMANDS (TIMELINE) - [NEW]
    // =============================================================

    class CYCLOPS_API DuplicateKeyframeCommand : public Command
    {
    public:
        DuplicateKeyframeCommand(Canvas* canvas, int layerIndex, int frameIndex);
        virtual std::string GetName() const override;
        virtual void Execute() override;
        virtual void Undo() override;
    private:
        Canvas* m_Canvas;
        int m_LayerIndex;
        int m_FrameIndex;
    };

    class CYCLOPS_API RemoveKeyframeCommand : public Command
    {
    public:
        RemoveKeyframeCommand(Canvas* canvas, int layerIndex, int frameIndex);
        virtual std::string GetName() const override;
        virtual void Execute() override;
        virtual void Undo() override;
    private:
        Canvas* m_Canvas;
        int m_LayerIdx, m_FrameIdx;
        std::shared_ptr<Framebuffer> m_Backup; // Stores pixel data
    };

    class CYCLOPS_API MoveKeyframeCommand : public Command
    {
    public:
        MoveKeyframeCommand(Canvas* canvas, int layerIdx, int fromFrame, int toFrame);
        virtual std::string GetName() const override;
        virtual void Execute() override;
        virtual void Undo() override;
    private:
        Canvas* m_Canvas;
        int m_LayerIdx, m_From, m_To;
        bool m_OverwroteDest = false; // Did we kill an existing frame?
        std::shared_ptr<Framebuffer> m_DestBackup; // Backup if we overwrote something
    };
}