BASENAME=poster
#BASENAME=poster_landscape

$(BASENAME).pdf: $(BASENAME).tex 
	pdflatex $(BASENAME)
	pdflatex $(BASENAME)
	pdflatex $(BASENAME)

clean:
	rm -f $(BASENAME).aux	$(BASENAME).bbl	$(BASENAME).blg	$(BASENAME).log	*~
