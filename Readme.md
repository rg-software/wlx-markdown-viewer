# WLX Markdown Viewer
Markdown lister plugin for Total Commander (32/64-bit version)

The plugin is based on [HTMLView 1.2.6 source](http://sites.google.com/site/htmlview/). It converts markdown files into HTML, then invokes HTMLView to display the resulting document. The processing is performed via [hoedown library](https://github.com/hoedown/hoedown). All operations are done in memory without any temporary files. External resources such as images (specified with relative links) are supported. The plugin should work reliably for UTF-8 and UTF-16 encoded files. For other formats, Internet Explorer-based autodetection is used, so results may vary. File format is detected with [text-encoding-detect library](https://github.com/AutoItConsulting/text-encoding-detect).

# Fine Tuning
Plugin configuration is specified in `MarkdownView.ini`. Markdown-related settings are:

* `MarkdownExtensions`: file extensions recognized by the plugin as markdown files.
* `HoedownArgs`: custom arguments passed to the hoedown engine (control fine settings of markdown rendering).
* `UseSmartypants`: controls whether `smartypants` should be invoked after processing.
* `CustomCSS`: a path to a custom CSS file for customizing the resulting look of the document. A collection of four CSS files from [Markdown CSS](http://markdowncss.github.io/) is included into the package.

For more information on `hoedown` arguments and `smartypants` functionality, check [this quick reference](http://htmlpreview.github.com?https://github.com/rg-software/wlx-markdown-viewer/blob/master/hoedown.html).

# Setup
The binary plugin archive comes with the setup script. Just enter the archive, and confirm installation.

# Development
The current project file can be compiled with Visual Studio 2017. For the sake of Windows XP support, `141_xp` toolset is used (enable `Windows XP support for C++` option in the optional components of the `Desktop development with C++` workload to install it). Note that `/Zc:threadSafeInit-` compiler switch is needed for Windows XP only, so we can remove it in the future.