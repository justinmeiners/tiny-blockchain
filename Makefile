.PHONY: all main clean

all: source docs/index.html

source:
	srcweave --tangle docs/src/ index.lit

docs/index.html: index.lit
	srcweave --format srcweave-format --weave docs/ $<

clean:
	rm -f docs/src/blockchain.c
	rm -f docs/src/lc3-alt.cpp
	rm -f docs/index.html
	rm -f docs/main.css
