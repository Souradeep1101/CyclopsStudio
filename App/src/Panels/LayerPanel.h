#pragma once

#include "Cyclops/Scene/Canvas.h"

namespace Cyclops {

	class LayerPanel
	{
	public:
		LayerPanel() = default;

		// The main render function called by EditorLayer
		void OnImGuiRender(Canvas* canvas);

	private:
		void DrawLayerNode(Canvas* canvas, int index);
	};

}