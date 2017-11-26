# de-law-explorer

An experimental tool to assist working on law codes. For demonstratioinal
purposes, it works on federal law of Germany.

Notable features include:

* Overall improved representation of text.
* High precision wiki-linking between books and codes.
* Inline code inspection.
* Context visualization.
* High precision bidirectional references.
* Work stack.

This tool served as prototype to evaluate efficiency increases on 
lawyers' work.

## Impressions

Code viewer:

![Code viewer](https://raw.github.com/MarcelHB/de-law-explorer/master/screenshots/1.png)

Neighbours for SGB 3:

![Code neighbouring](https://raw.github.com/MarcelHB/de-law-explorer/master/screenshots/2.png)

Relation between SGB 3 and HwO:

![Code relations](https://raw.github.com/MarcelHB/de-law-explorer/master/screenshots/3.png)

## Disclaimer

* This is a purely experimental tool, serving demonstrational purposes only.
* This project is no longer under active development, and has not been
  after 2015. Feature may be incomplete or crash.
* The UI is mostly German due to its initial target audience.
* It has not been reviewed for security issues. Neither this code nor
  any of its last dependencies. Use at your own risk!
* We do not accept pull-requests, treat issues, ship releases or do any kind
  of support. Feel free to fork this repository on your own behalf.
* **Do not** use this tool for production, it contains known flaws (see below)
* We **do not** share this as a service subject to German code of law services
  (*Rechtsdienstleistungsgesetz*).
* This project's build dependencies (not included) might or might not be compatible
  to GPL v3.
* The build chain of this project might or might not generate license compliant
  outputs with respect to dependencies. Be aware in case of distribution.

## Setup

Grab Qt SDK v5.4 (newer versions not supported for `webview/webwidgets/...`
incompatiblity). You need a C++ compiler with 2014 versions of GNU or
clang being known to work.

You need these additional dependencies:

* FreeType2 v2.5.3
* Glew
* GLM
* jQuery v1.11
* 7z

Put these dependencies to the locations according to `de_law_explorer.pro`
project file. Let Qt Creator build the project for you.

We have tested successful builds on Windows, Mac and Linux, given a OpenGL v2.1
compatible GPU.

For doing a complete setup, look at `parser/README.md`.

## Flaws and limitations

Parsing laws is non-trivial. Especially on the lack on non-semantic
schema or formats. We have tested parsing German federal law, and
estimate the accuracy of trivial content (non-trivial contentent is images for
not being supported and unknown copyrights, tables for mostly being irregular,
technical descriptions, schemas, ...) up to 90%.

The missing 10% make the tool worthless for silently swallowing or
messing up things. We estimate a demand for human curating for finding and
fixing such issues under constant updates of new laws.

## Authors

Marcel Heing-Becker, Timo Rademann

## License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See `LICENSE`
for more details.
