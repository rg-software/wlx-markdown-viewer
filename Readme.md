# WLX Markdown Viewer

HTML + Markdown lister plugin for Total Commander (32/64-bit version)

Based on [HTMLView 1.2.6 source](http://sites.google.com/site/htmlview/), this plugin supports the original HTMLView functionality and displays Markdown files via [hoedown](https://github.com/hoedown/hoedown). Markdown rendering should work reliably for UTF-8 and UTF-16 encoded files (file format is detected with [text-encoding-detect](https://github.com/AutoItConsulting/text-encoding-detect)). For other encodings, Internet Explorer-based autodetection is used, so results may vary.

## Fine Tuning

Plugin configuration is specified in `MarkdownView.ini`. Markdown-related settings are:

* `MarkdownExtensions`: file extensions recognized by the plugin as markdown files.
* `HoedownArgs`: custom arguments passed to the hoedown engine (control fine settings of markdown rendering).
* `UseSmartypants`: controls whether `smartypants` should be invoked after processing.
* `CustomCSS`: a path to a CSS sheet for customizing the resulting look of the document. A collection of four sheets from [Markdown CSS](https://markdowncss.github.io/) and six Github-inspired sheets courtesy of S.&nbsp;Kuznetsov is included into the package.

For more information on `hoedown` arguments and `smartypants` functionality, check [this quick reference](https://htmlpreview.github.io?https://raw.githubusercontent.com/rg-software/wlx-markdown-viewer/master/hoedown.html).

## Internet Explorer Update

The plugin is based on an obsolete Internet Explorer engine, which can be upgraded via [registry hacks](https://github.com/rg-software/wlx-markdown-viewer/raw/master/ie_upgrade_registry.zip) (check [MSDN](https://learn.microsoft.com/en-us/previous-versions/windows/internet-explorer/ie-developer/general-info/ee330730(v=vs.85)?redirectedfrom=MSDN#browser-emulation) for details.)

## Setup

The binary plugin archive comes with the setup script. Just enter the archive, and confirm installation.

## Development

The current project file can be compiled with Visual Studio 2022. However, if Windows XP support is needed, Visual Studio 2019 version 16.7 or earlier [must be used](https://learn.microsoft.com/en-us/cpp/build/configuring-programs-for-windows-xp?view=msvc-170#windows-xp-deployment). In any case, the code is currently compiled with `141_xp` toolset (enable `Windows XP support for C++` option in the optional components of the `Desktop development with C++` workload to install it). Note that `/Zc:threadSafeInit-` compiler switch is needed for Windows XP.
