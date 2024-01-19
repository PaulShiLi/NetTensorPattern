CC = gcc
Flags = --static -O3
LD = $(CC)
SRCS := $(filter-out src/c/Tensor_Python.c, $(wildcard src/c/*.c))
OBJS := $(patsubst src/c/%.c, build/%.o, $(SRCS))

all: NetworkTensorPattern

python:
	python setup.py pnt --compile;
	pip3 install .;
	# python test/test.py;

install:
	python setup.py install;

uninstall:
	pip uninstall NetworkTensorPattern; 

build/%.o: src/c/%.c
	$(CC) $(Flags) -c $< -o $@

NetworkTensorPattern: $(OBJS)
	$(LD) $(Flags) $^ -o $@ -lm

run:
	bash ./test_run.sh

clean:
	rm -rf NetworkTensorPattern \
	**.so \
	src/nettensorpat/*.so \
	build/* \
	**.egg-info \
	src/*.egg-info \
	dist \
	src/*__pycache__ \
	temp \
	**__pycache__; \
	yes | pip uninstall nettensorpat;
