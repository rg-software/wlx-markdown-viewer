# WLX Markdown Viewer
Markdown lister plugin for Total Commander (64-bit version)

The plugin is based on [HTMLView 1.2.6 source](http://sites.google.com/site/htmlview/). It converts markdown files into HTML, then invokes HTMLView to display the resulting document. The processing is performed via [hoedown](https://github.com/hoedown/hoedown).

# Installation
The binary plugin archive comes with the setup script. Just enter the archive, and confirm installation.

# Development
Currently, only files with `.markdown`, `.mk`, and `.md` extensions are processed (they are hardcoded in `main.cpp`). The supported extensions should be specified in the configuration file instead.

The resulting HTML file is placed into a temporary folder, then HTMLView is invoked to display it. It means that no resources specified with relative links (such as images) can be shown correctly. This limitation should be removed in the future.
