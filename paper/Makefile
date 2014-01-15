NAME=paper

TARGET=$(NAME)-cos518
BIBTEX := bibtex
TGIF   := tgif
XFIG   := xfig
GNUPLOT:= gnuplot

SOURCES=$(NAME)-cos518.tex \
	abstract.tex \
	intro.tex \
	background.tex \
    arch.tex \
	eval.tex \
	related.tex \
	conclusion.tex \
    costmodel.tex \
	local.bib

FIGS= figs/conventional-hierarchical-topo.pdf \
      figs/fat-tree-topo.pdf \
      figs/switchless-arch-torus.pdf \
      figs/switchless-arch-cube.pdf \
      figs/in-server-switch-bus.pdf \
      figs/in-server-switch-noc.pdf \
      figs/dimension_ordered_udp.pdf \
      figs/nbody_delay.pdf \
      figs/nbody_latency.pdf \
      figs/Weather_cube-dimordered_50.log.parsed_delay.pdf \
      figs/Weather_cube-dimordered_delay.pdf
      

all: $(TARGET).ps
pdf: all

$(TARGET).pdf: Makefile  $(SOURCES) $(FIGS)
	pdflatex  $(TARGET).tex
	-bibtex --min-crossrefs=100    $(TARGET)
	pdflatex  $(TARGET).tex
	pdflatex  $(TARGET).tex

color: $(TARGET).pdf
	pdflatex $(TARGET).tex
	pdftops $(TARGET).pdf

$(TARGET).ps: $(TARGET).pdf
	pdflatex "\newcommand{\wantBW}{} \input{$(TARGET).tex}"
	pdftops $(TARGET).pdf

%.pdf : %.fig #Makefile
	fig2dev -L pdf -b 1 $< $@

%.eps : %.dia #Makefile
	dia --nosplash -e $@ $<

%.eps : %.obj
	TMPDIR=/tmp $(TGIF) -print -eps $<

%.pdf : %.eps #Makefile
	epstopdf $<

display: $(TARGET).pdf
	evince $(TARGET).pdf

clean:
	rm -f *.aux *.log *.out *.bbl *.blg *~ *.bak $(TARGET).ps $(TARGET).pdf
