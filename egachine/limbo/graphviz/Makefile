all: example.ps

%.ps : %.dot
	dot -Tps $< > $@
	see $@


