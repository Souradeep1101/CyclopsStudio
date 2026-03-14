#pragma once

#include "Cyclops/Core/Base.h"

#include <string>

namespace Cyclops
{
	class CYCLOPS_API Command
	{
	public:
		virtual ~Command() = default;

		// The name is useful for debugging (e.g., "Undo Paint Stroke")
		virtual std::string GetName() const = 0;

		// The core logic
		virtual void Execute() = 0; // Also serves as "Redo"
		virtual void Undo() = 0;

		// Memory Management:
		// Some commands (like paint strokes) take up a lot of VRAM.
		// We might need a method later to "Merge" commands or free snapshots.
		virtual size_t GetSizeInMemory() const { return 0; }
	};
}