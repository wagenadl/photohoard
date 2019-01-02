ALL: grandscheme.pdf

%.pdf: %.tex
	pdflatex $<

%.tex: %.ptx
	pretex $< > $@
