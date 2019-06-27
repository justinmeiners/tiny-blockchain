all: docs/src/blockchain.c docs/index.html

docs/src/blockchain.c docs/src/blockchain.cpp: index.lit
	lit --tangle $<
	mv blockchain.c docs/src/

docs/index.html: index.lit main.css
	lit --weave $<
	mv index.html docs/

.PHONY:
clean:
	rm -f docs/src/blockchain.c
	rm -f docs/src/lc3-alt.cpp
	rm -f docs/index.html
	rm -f docs/main.css
