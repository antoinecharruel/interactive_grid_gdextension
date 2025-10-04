# README

## Build command Pandoc

```bash
pandoc demo.md -o demo.pdf --pdf-engine=xelatex -V mainfont="Hack" --variable=geometry:margin=1in --variable=fontsize=10pt --variable=pagestyle=plain --include-in-header=custom.tex
```