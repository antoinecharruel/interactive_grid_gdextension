# README

## Build command Pandoc

```bash
pandoc demo.md -o demo.pdf --pdf-engine=xelatex -V mainfont="DejaVu Sans" --variable=geometry:margin=1in --variable=fontsize=10pt --variable=pagestyle=plain --include-in-header=custom.tex


pandoc demo_utf8.md -o demo.pdf --pdf-engine=xelatex -V mainfont="DejaVu Sans" --variable=geometry:margin=1in --variable=fontsize=10pt --variable=pagestyle=plain --include-in-header=custom.tex


pandoc demo.md -o demo.pdf --pdf-engine=xelatex -V --include-in-header=custom.tex
```

```bash
pandoc -s data/data.csv -o data/data.md
```
