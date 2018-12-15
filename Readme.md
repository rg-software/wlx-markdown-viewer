# WLX Markdown Viewer
Markdown lister plugin for Total Commander (32/64-bit version)

The plugin is based on [HTMLView 1.2.6 source](http://sites.google.com/site/htmlview/). It converts markdown files into HTML, then invokes HTMLView to display the resulting document. The processing is performed via [hoedown library](https://github.com/hoedown/hoedown). All operations are performed in memory without any temporary files.

# Fine Tuning
Check `MarkdownView.ini` for options. Markdown-specific settings are:

* `MarkdownExtensions`: file extensions recognized by the plugin as markdown files.
* `HoedownArgs`: custom arguments passed to hoedown engine (control fine settings of markdown rendering).
* `UseSmartypants`: controls whether `smartypants` should be invoked after processing.
* `CustomCSS`: a path to a custom CSS file for customizing the resulting look of the document. A collection of four CSS files from [Markdown CSS](http://markdowncss.github.io/) is included into the package.

For more information on `hoedown` arguments and `smartypants` functionality, check [this quick reference](hoedown.html).

# Installation
The binary plugin archive comes with the setup script. Just enter the archive, and confirm installation.

# Limitations
No external resources (such as images) can be shown.
