ifeq ($(OS),Windows_NT)
    PY = python
else
    PY = python3
endif

all: dist

build:
	$(PY) setup.py build

dist:
	$(PY) setup.py sdist bdist_wheel

install:
	$(PY) -m pip install .

test: build
	$(PY) test.py

upload:
	$(PY) -m twine upload dist/*

clean:
	$(RM) -r build dist *.egg-info *.dist-info __pycache__ *.pyc *.pyd *.so

.PHONY: all build dist install test upload clean
