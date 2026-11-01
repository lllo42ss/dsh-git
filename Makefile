PYTHON ?= python

.PHONY: test check zip clean

test:
	$(PYTHON) -B -m unittest discover -s tests -v

check:
	$(PYTHON) -B scripts/check_repo.py

zip:
	mkdir -p dist
	git archive --format=zip --output=dist/dsh-git-$(shell git describe --tags --always).zip HEAD

clean:
	rm -rf dist
