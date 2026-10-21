#ifndef _SETTINGS_DEFAULTS_H_
#define _SETTINGS_DEFAULTS_H_

// Shipped defaults of the plugin configuration - the third resolution layer.
//
// They mirror the *active* values of Build/MarkdownView.ini, which is the
// source of truth for the plugin configuration (the file ships as
// MarkdownView.ini.sample next to the plugin). Both must stay in sync:
// tests/ini_settings_test.cpp verifies that, the audit table in
// openspec/changes/2026-10-21-sample-ini-fallback/design.md documents them.
//
// Keys that are commented out in the sample are inactive and keep their own
// default.

// [options]
static const int DEFAULT_SHOW_DIRS = 1;
static const int DEFAULT_ALLOW_POPUPS = 0;
static const int DEFAULT_SHOW_STATUSBAR = 1;
static const int DEFAULT_SHOW_TOOLBAR = 1;
static const int DEFAULT_USE_SAVE_POSITION = 0;
static const int DEFAULT_SILENT = 1;
static const int DEFAULT_ALLOW_SCRIPTING = 0;
static const int DEFAULT_SHOW_IMAGES = 1;
static const int DEFAULT_SHOW_VIDEOS = 0;
static const int DEFAULT_PLAY_SOUNDS = 0;
static const int DEFAULT_ALLOW_JAVA = 0;
static const int DEFAULT_ALLOW_ACTIVEX = 1;
static const int DEFAULT_FORCE_OFFLINE = 1;
static const int DEFAULT_HIGHLIGHT_ALL_MATCHES = 0;
static const char* const DEFAULT_LISTER_TITLE = "";
static const char* const DEFAULT_NEW_WINDOW_HTML = "default";
static const char* const DEFAULT_NEW_WINDOW_CHM = "default";

// [Debug] - every key is commented out in the sample and therefore inactive
static const int DEFAULT_USE_MOZILLA_CONTROL = 0;
static const int DEFAULT_QUICK_QUIT = 0;
static const int DEFAULT_GLOBAL_HOOK = 0;
static const int DEFAULT_TOOLBAR_BPP = 2;
static const int DEFAULT_KEEP_HOOK_WHEN_NO_WINDOWS = 0;

// [Extensions]
static const int DEFAULT_SIGNATURE_SKIP_SPACES = 1;
static const char* const DEFAULT_CHM_EXTENSIONS = "chm";
static const char* const DEFAULT_HTML_EXTENSIONS = "htm;html;shtml;plg;mht;mhtml;xml;xsl;php;php2;php3;xhtml;pl;asp;htt";
static const char* const DEFAULT_MARKDOWN_EXTENSIONS = "md;markdown;mdown;mdtext;mdtxt;mdwn;mk;mkd;mkdn;mkdown";
static const char* const DEFAULT_SIGNATURES = "\"<HTML>\";\"<?xml\";\"<!DOCUMENT\";\"<!DOCTYPE HTML\"";

// [Hotkeys]
static const char* const DEFAULT_TRANSLATION_HOTKEYS = "1;2;3;4;5;6;7;N;P;F3;Shift+F3;F7;Esc;Tab;Ctrl+Q;Ctrl+P;Ctrl+A;Ctrl+C;Ctrl+Insert;Ctrl+F";
static const char* const DEFAULT_TYPING_TRANSLATION_HOTKEYS = "F3;Shift+F3;F7;Esc;Tab;Ctrl+Q;Ctrl+P;Ctrl+A;Ctrl+C;Ctrl+Insert;Ctrl+F";
// The hotkey actions are a dynamic key space (the key name is the hotkey), so
// there is no per key default: a key that is in neither file means "no action".
static const char* const DEFAULT_HOTKEY_ACTION = "";

// [Renderer]
static const char* const DEFAULT_HOEDOWN_ARGS = "hoedown --hard-wrap --all-block --autolink --strikethrough --highlight --superscript";
static const char* const DEFAULT_USE_SMARTYPANTS = "1";
static const char* const DEFAULT_CUSTOM_CSS = "css\\github.css";
static const char* const DEFAULT_CUSTOM_CSS_DARK = "css\\github.dark.css";

#endif //  _SETTINGS_DEFAULTS_H_
