#include "Cyclops/Core/CommandHistory.h"
#include <iostream>

namespace Cyclops
{
	std::vector<std::shared_ptr<Command>> CommandHistory::s_UndoStack;
	std::vector<std::shared_ptr<Command>> CommandHistory::s_RedoStack;

	void CommandHistory::AddCommand(std::shared_ptr<Command> cmd, bool execute)
	{
		// 1. The "Safety Net" Logic
		// Most UI actions (Buttons) need to run immediately.
		// Paint actions are already done, so they pass 'false'.
		if (execute)
		{
			cmd->Execute();
		}

		// 2. Clear Redo because the timeline has branched
		s_RedoStack.clear();

		// 3. Push to Undo Stack
		s_UndoStack.push_back(cmd);

		// 4. Limit History Size
		if (s_UndoStack.size() > MAX_HISTORY_SIZE)
		{
			s_UndoStack.erase(s_UndoStack.begin());
		}

		std::cout << "[History] Added: " << cmd->GetName() << " (Stack: " << s_UndoStack.size() << ")" << std::endl;
	}

	void CommandHistory::Undo()
	{
		if (s_UndoStack.empty()) return;

		// 1. Get the last command
		auto cmd = s_UndoStack.back();
		s_UndoStack.pop_back();

		// 2. Move to Redo Stack
		s_RedoStack.push_back(cmd);

		// 3. Perform Undo
		std::cout << "[History] Undoing: " << cmd->GetName() << std::endl;
		cmd->Undo();
	}

	void CommandHistory::Redo()
	{
		if (s_RedoStack.empty()) return;

		// 1. Get the command to redo
		auto cmd = s_RedoStack.back();
		s_RedoStack.pop_back();

		// 2. Move back to Undo Stack
		s_UndoStack.push_back(cmd);

		// 3. Perform Redo
		std::cout << "[History] Redoing: " << cmd->GetName() << std::endl;
		cmd->Execute();
	}

	bool CommandHistory::CanUndo()
	{
		return !s_UndoStack.empty();
	}

	bool CommandHistory::CanRedo()
	{
		return !s_RedoStack.empty();
	}

	void CommandHistory::Clear()
	{
		s_UndoStack.clear();
		s_RedoStack.clear();
	}
}