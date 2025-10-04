#pragma once
#include <string>
#include "TextEditor.h"  // Include ImGuiColorTextEdit

struct CodeEditor {
    TextEditor editor;        // The syntax-highlighted editor
    bool isOpen = true;       // Toggle window open/close

    // Constructor to set up the editor
    CodeEditor() {
        // Set Lua syntax highlighting
        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());

        // Set dark color scheme
        editor.SetPalette(TextEditor::GetDarkPalette());

        // Enable features
        editor.SetShowWhitespaces(false);
    }
};

void RenderCodeEditor(CodeEditor& editor, const char* title = "Lua Editor");