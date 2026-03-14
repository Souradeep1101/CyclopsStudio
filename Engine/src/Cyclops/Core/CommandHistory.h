#pragma once

#include "Cyclops/Core/Command.h"
#include "Cyclops/Core/Base.h"

#include <vector>
#include <memory>
#include <stack>

namespace Cyclops
{
	class CYCLOPS_API CommandHistory
	{
	public:
		// Add a new command and execute it immediately
		// (Used when the user performs an action)
		static void AddCommand(std::shared_ptr<Command> cmd, bool execute = true);

		static void Undo();
		static void Redo();

		// Helper to check if buttons should be enabled
		static bool CanUndo();
		static bool CanRedo();

		static void Clear();

	private:
		// We use a vector as a stack so we can iterate if needed
		static std::vector<std::shared_ptr<Command>> s_UndoStack;
		static std::vector<std::shared_ptr<Command>> s_RedoStack;

		// Limits (Optional safety against infinite memory usage)
		static const size_t MAX_HISTORY_SIZE = 50;
	};
}