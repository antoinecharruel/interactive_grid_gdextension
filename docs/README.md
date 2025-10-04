# README

## Build command Pandoc

```bash
pandoc demo.md -o ../addons/interactive-grid/demo.pdf --pdf-engine=xelatex -V mainfont="Hack" --variable=geometry:margin=1in --variable=fontsize=10pt --variable=pagestyle=plain --include-in-header=custom.tex

cp ../addons/interactive-grid/demo.pdf ../demo/addons/interactive-grid/demo.pdf
```

## Build doc interactiveGrid.xml
```bash
cd demo
godot --doctool .. --gdextension-docs
& "C:\Program Files (x86)\Steam\steamapps\common\Godot Engine\godot.windows.opt.tools.64.exe" --doctool .. --gdextension-docs
```