#pragma once

#include "Cyclops/Renderer/Framebuffer.h"
#include "Cyclops/Scene/CanvasLayer.h"
#include "Cyclops/Core/Base.h"

#include <vector>
#include <memory>
#include <string>

namespace Cyclops
{
    class CYCLOPS_API Canvas
    {
    public:
        Canvas(uint32_t width, uint32_t height);

        // --- Layer Management ---
        void AddLayer(const std::string& name = "New Layer");
        void RemoveLayer(int index); // [KEPT THIS ONE]
        void InsertLayer(int index, const CanvasLayer& layer);
        void MoveLayer(int fromIndex, int toIndex);
        void SetActiveLayer(int index);

        // Helper to check if we can delete (don't delete the last layer!)
        bool CanDeleteLayer() const { return m_Layers.size() > 1; }

        // --- Framebuffer Access ---
        // Helper now needs to know the time!
        Framebuffer* GetActiveFramebuffer();

        // --- Time Control ---
        void SetCurrentFrame(int frameIndex);
        int GetCurrentFrame() const { return m_CurrentFrame; }

        // --- System ---
        void Resize(uint32_t width, uint32_t height);
        void Recompose(); // Redraws the composite based on layers + time

        // --- Getters ---
        uint32_t GetTextureID() const { return m_CompositeBuffer->GetTextureID(); }
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

        const std::vector<CanvasLayer>& GetLayers() const { return m_Layers; }
        std::vector<CanvasLayer>& GetLayers() { return m_Layers; }

        int GetActiveLayerIndex() const { return m_ActiveLayerIndex; }

        // --- Onion Skin Controls ---
        void SetOnionSkinEnabled(bool enabled);
        bool IsOnionSkinEnabled() const { return m_OnionSkinEnabled; }

    private:
        uint32_t m_Width, m_Height;
        std::shared_ptr<Framebuffer> m_CompositeBuffer;

        std::vector<CanvasLayer> m_Layers;
        int m_ActiveLayerIndex = 0;

        // The Timeline Cursor
        int m_CurrentFrame = 0;
        bool m_OnionSkinEnabled = true;
    };
}