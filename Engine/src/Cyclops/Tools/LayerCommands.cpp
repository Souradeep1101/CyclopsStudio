#include "Cyclops/Tools/LayerCommands.h"

namespace Cyclops {

    // =============================================================
    // ADD LAYER
    // =============================================================

    AddLayerCommand::AddLayerCommand(Canvas* canvas)
        : m_Canvas(canvas) {
    }

    std::string AddLayerCommand::GetName() const { return "Add Layer"; }

    void AddLayerCommand::Execute()
    {
        m_Canvas->AddLayer();
        m_AddedIndex = (int)m_Canvas->GetLayers().size() - 1;
        m_Canvas->Recompose();
    }

    void AddLayerCommand::Undo()
    {
        m_Canvas->RemoveLayer(m_AddedIndex);
        m_Canvas->Recompose();
    }

    // =============================================================
    // REMOVE LAYER
    // =============================================================

    RemoveLayerCommand::RemoveLayerCommand(Canvas* canvas, int index)
        : m_Canvas(canvas), m_Index(index)
    {
        // Capture state immediately upon creation
        if (index >= 0 && index < m_Canvas->GetLayers().size())
            m_BackupLayer = m_Canvas->GetLayers()[index];
    }

    std::string RemoveLayerCommand::GetName() const { return "Remove Layer"; }

    void RemoveLayerCommand::Execute()
    {
        m_Canvas->RemoveLayer(m_Index);
        m_Canvas->Recompose();
    }

    void RemoveLayerCommand::Undo()
    {
        m_Canvas->InsertLayer(m_Index, m_BackupLayer);
        m_Canvas->Recompose();
    }

    // =============================================================
    // MOVE LAYER
    // =============================================================

    MoveLayerCommand::MoveLayerCommand(Canvas* canvas, int fromIndex, int toIndex)
        : m_Canvas(canvas), m_From(fromIndex), m_To(toIndex) {
    }

    std::string MoveLayerCommand::GetName() const { return "Reorder Layer"; }

    void MoveLayerCommand::Execute()
    {
        m_Canvas->MoveLayer(m_From, m_To);
        m_Canvas->Recompose();
    }

    void MoveLayerCommand::Undo()
    {
        m_Canvas->MoveLayer(m_To, m_From);
        m_Canvas->Recompose();
    }

    // =============================================================
    // DUPLICATE KEYFRAME (TIMELINE)
    // =============================================================

    DuplicateKeyframeCommand::DuplicateKeyframeCommand(Canvas* canvas, int layerIndex, int frameIndex)
        : m_Canvas(canvas), m_LayerIndex(layerIndex), m_FrameIndex(frameIndex) {
    }

    std::string DuplicateKeyframeCommand::GetName() const { return "Duplicate Keyframe"; }

    void DuplicateKeyframeCommand::Execute()
    {
        auto& layer = m_Canvas->GetLayers()[m_LayerIndex];
        layer.DuplicateKeyframe(m_FrameIndex);
        m_Canvas->Recompose();
    }

    void DuplicateKeyframeCommand::Undo()
    {
        auto& layer = m_Canvas->GetLayers()[m_LayerIndex];
        layer.RemoveKeyframe(m_FrameIndex);
        m_Canvas->Recompose();
    }

    // =============================================================
    // REMOVE KEYFRAME (TIMELINE)
    // =============================================================

    RemoveKeyframeCommand::RemoveKeyframeCommand(Canvas* canvas, int layerIndex, int frameIndex)
        : m_Canvas(canvas), m_LayerIdx(layerIndex), m_FrameIdx(frameIndex)
    {
        auto& layer = m_Canvas->GetLayers()[m_LayerIdx];
        if (layer.Keyframes.count(m_FrameIdx))
        {
            // [IMPORTANT] Deep Copy (Clone) for the backup!
            m_Backup = layer.Keyframes[m_FrameIdx]->Data->Clone();
        }
    }

    std::string RemoveKeyframeCommand::GetName() const { return "Remove Keyframe"; }

    void RemoveKeyframeCommand::Execute()
    {
        m_Canvas->GetLayers()[m_LayerIdx].RemoveKeyframe(m_FrameIdx);
        m_Canvas->Recompose();
    }

    void RemoveKeyframeCommand::Undo()
    {
        if (m_Backup) {
            auto& layer = m_Canvas->GetLayers()[m_LayerIdx];
            // Restore from backup
            layer.Keyframes[m_FrameIdx] = std::make_shared<Keyframe>(m_FrameIdx, m_Backup->Clone());
        }
        m_Canvas->Recompose();
    }

    // =============================================================
    // MOVE KEYFRAME (TIMELINE)
    // =============================================================

    MoveKeyframeCommand::MoveKeyframeCommand(Canvas* canvas, int layerIdx, int fromFrame, int toFrame)
        : m_Canvas(canvas), m_LayerIdx(layerIdx), m_From(fromFrame), m_To(toFrame)
    {
        // Check if we are going to overwrite a frame at the destination
        auto& layer = m_Canvas->GetLayers()[m_LayerIdx];
        if (layer.Keyframes.count(m_To))
        {
            m_OverwroteDest = true;
            m_DestBackup = layer.Keyframes[m_To]->Data->Clone();
        }
    }

    std::string MoveKeyframeCommand::GetName() const { return "Move Keyframe"; }

    void MoveKeyframeCommand::Execute()
    {
        auto& layer = m_Canvas->GetLayers()[m_LayerIdx];
        if (layer.Keyframes.find(m_From) == layer.Keyframes.end()) return;

        // 1. Get Source Shared Ptr
        auto keyframe = layer.Keyframes[m_From];

        // 2. Remove Source
        layer.RemoveKeyframe(m_From);

        // 3. Update the keyframe's internal index
        keyframe->FrameIndex = m_To;

        // 4. Place at Dest
        layer.Keyframes[m_To] = keyframe;

        m_Canvas->Recompose();
    }

    void MoveKeyframeCommand::Undo()
    {
        auto& layer = m_Canvas->GetLayers()[m_LayerIdx];

        // 1. Take it back from Dest
        if (layer.Keyframes.find(m_To) == layer.Keyframes.end()) return;
        auto keyframe = layer.Keyframes[m_To];
        layer.RemoveKeyframe(m_To);

        // 2. Put it back at Source
        keyframe->FrameIndex = m_From;
        layer.Keyframes[m_From] = keyframe;

        // 3. Restore the overwritten frame (if any)
        if (m_OverwroteDest && m_DestBackup)
        {
            layer.Keyframes[m_To] = std::make_shared<Keyframe>(m_To, m_DestBackup->Clone());
        }

        m_Canvas->Recompose();
    }
}